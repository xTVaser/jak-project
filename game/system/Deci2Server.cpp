/*!
 * @file Deci2Server.cpp
 * Basic implementation of a DECI2 server.
 * Works with deci2.cpp (sceDeci2) to implement the networking on target
 */

#include "Deci2Server.h"

#include "common/listener_common.h"
#include "common/log/log.h"
#include "common/util/Assert.h"
#include "common/versions/versions.h"

#include "third-party/fmt/core.h"

Deci2Server::~Deci2Server() {
  // Cleanup the accept thread
  if (accept_thread_running) {
    kill_accept_thread = true;
    // NOTE - if we don't want to wait for the roundtrip timeout to exit the game gracefully
    // we should just terminate the thread forcefully
    accept_thread.join();
    accept_thread_running = false;
  }
  accepted_socket.close();
}

void Deci2Server::post_init() {
  lg::info("[Deci2Server:{}] awaiting connections", tcp_port);
  accept_thread_running = true;
  kill_accept_thread = false;
  accept_thread = std::thread(&Deci2Server::accept_thread_func, this);
}

void Deci2Server::accept_thread_func() {
  while (!kill_accept_thread) {
    std::chrono::milliseconds timeout(100);
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    sockpp::inet_address peer;
    while (std::chrono::steady_clock::now() < deadline) {
      accepted_socket = acceptor.accept(&peer);
      if (accepted_socket) {
        break;
      }
    }
    if (accepted_socket) {
      lg::info("[DECI2:{}]: Received connection from {}", tcp_port, peer.to_string());
      accepted_socket.write_timeout(std::chrono::microseconds(100000));  // TODO - check error
      u32 versions[2] = {versions::GOAL_VERSION_MAJOR, versions::GOAL_VERSION_MINOR};
      accepted_socket.write_n((char*)&versions, 8);  // TODO - check error
      client_connected = true;
      return;
    }
  }
}

bool Deci2Server::is_client_connected() {
  return client_connected;
}

/*!
 * Wait for protocols to become ready.
 * This avoids the case where we receive messages before protocol handlers are set up.
 */
bool Deci2Server::wait_for_protos_ready() {
  if (protocols_ready || want_shutdown) {
    return !want_shutdown;
  }
  std::unique_lock<std::mutex> lk(server_mutex);
  cv.wait(lk, [&] { return protocols_ready || want_shutdown; });
  return !want_shutdown;
}

void Deci2Server::send_shutdown() {
  lock();
  want_shutdown = true;
  unlock();
  cv.notify_all();
}

/*!
 * Inform server that protocol handlers are ready.
 * Will unblock wait_for_protos_ready and incoming messages will be dispatched to these
 * protocols.  You can change the protocol handlers, but you should lock the mutex before
 * doing so.
 */
void Deci2Server::send_proto_ready(Deci2Driver* drivers, int* driver_count) {
  lock();
  d2_drivers = drivers;
  d2_driver_count = driver_count;
  protocols_ready = true;
  unlock();
  cv.notify_all();
}

void Deci2Server::read_data() {
  if (!is_client_connected()) {
    return;
  }

  int bytes_read = 0;
  int desired_size = (int)sizeof(Deci2Header);

  while (bytes_read != desired_size) {
    bytes_read = accepted_socket.read_n(buffer.data(), desired_size);  // TODO check error
    if (want_exit_callback()) {
      return;
    }
  }

  auto* hdr = (Deci2Header*)(buffer.data());
  lg::debug("[DECI2:{}]: Got message: {} {} {:x} {} -> {}", tcp_port, hdr->len, hdr->rsvd,
            hdr->proto, hdr->src, hdr->dst);

  hdr->rsvd = bytes_read;

  // see what protocol we got:
  lock();

  int handler = -1;
  for (int i = 0; i < *d2_driver_count; i++) {
    auto& prot = d2_drivers[i];
    if (prot.active && prot.protocol) {
      if (handler != -1) {
        lg::warn("[DECI2:{}] Warning: more than on protocol handler for this message!", tcp_port);
      }
      handler = i;
    }
  }

  if (handler == -1) {
    lg::warn("[DECI2] Warning: no handler for this message, ignoring...", tcp_port);
    unlock();
    return;
  }

  auto& driver = d2_drivers[handler];

  u32 sent_to_program = 0;
  while (!want_exit_callback() && (hdr->rsvd < hdr->len || sent_to_program < hdr->rsvd)) {
    // send what we have to the program
    if (sent_to_program < hdr->rsvd) {
      //      driver.next_recv_size = 0;
      //      driver.next_recv = nullptr;
      driver.recv_buffer = buffer.data() + sent_to_program;
      driver.available_to_receive = hdr->rsvd - sent_to_program;
      (driver.handler)(DECI2_READ, driver.available_to_receive, driver.opt);
      //      memcpy(driver.next_recv, buffer + sent_to_program, driver.next_recv_size);
      sent_to_program += driver.recv_size;
    }

    // receive from network
    int bytes = accepted_socket.read_n(buffer.data() + hdr->rsvd,
                                       hdr->len - hdr->rsvd);  // TODO check error
    if (want_exit_callback()) {
      return;
    }
    hdr->rsvd += bytes;
  }

  (driver.handler)(DECI2_READDONE, 0, driver.opt);
  unlock();
}

void Deci2Server::send_data(void* buf, u16 len) {
  lock();
  if (!client_connected) {
    lg::warn("[DECI2:{}] send while not connected, not sending!", tcp_port);
  } else {
    accepted_socket.write_n(buf, len);  // TODO - check error
    if (!client_connected || want_exit_callback()) {
      unlock();
      return;
    }
  }
  unlock();
}

void Deci2Server::lock() {
  server_mutex.lock();
}

void Deci2Server::unlock() {
  server_mutex.unlock();
}
