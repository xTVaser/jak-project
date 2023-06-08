
#include "ReplServer.h"

#include "common/cross_sockets/XTCPSocketServer.h"
#include "common/log/log.h"
#include "common/versions/versions.h"

#include "third-party/fmt/core.h"

// TODO - The server needs to eventually return the result of the evaluation
ReplServer::~ReplServer() {
  // Cleanup the accept thread
  if (accept_thread_running) {
    kill_accept_thread = true;
    // NOTE - if we don't want to wait for the roundtrip timeout to exit the game gracefully
    // we should just terminate the thread forcefully
    accept_thread.join();
    accept_thread_running = false;
  }
  // Close all our client sockets!
  for (auto& sock : client_sockets) {
    sock->close();
  }
}

void ReplServer::post_init() {
  // Add the listening socket to our set of sockets
  lg::info("[nREPL:{}:{}] awaiting connections", tcp_port, acceptor.address().to_string());
  accept_thread_running = true;
  kill_accept_thread = false;
  accept_thread = std::thread(&ReplServer::accept_thread_func, this);
}

bool ReplServer::ping_response(std::unique_ptr<sockpp::tcp_socket>& socket) {
  std::string ping = fmt::format("Connected to OpenGOAL v{}.{} nREPL!",
                                 versions::GOAL_VERSION_MAJOR, versions::GOAL_VERSION_MINOR);
  const auto ok = socket->write(ping);
  if (ok == -1) {
    lg::info("[nREPL:{}] Client Disconnected: {}", tcp_port, socket->address().to_string());
    socket->close();
    return false;
  }
  return true;
}

void ReplServer::accept_thread_func() {
  while (!kill_accept_thread) {
    std::chrono::milliseconds timeout(100);
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
      sockpp::inet_address peer;
      auto client_socket = acceptor.accept(&peer);
      if (client_socket) {
        auto sock_ptr = std::make_unique<sockpp::tcp_socket>(std::move(client_socket));
        client_socket.read_timeout(std::chrono::microseconds(100000));   // TODO - check error
        client_socket.write_timeout(std::chrono::microseconds(100000));  // TODO - check error
        const auto success = ping_response(sock_ptr);
        if (!success) {
          client_socket.close();
        } else {
          server_mutex.lock();
          if (client_sockets.size() == max_clients) {
            client_sockets.erase(client_sockets.begin());
          }
          client_sockets.push_back(std::move(sock_ptr));
          lg::info("[nREPL:{}]: Established connection to {}", tcp_port, peer.to_string());
          server_mutex.unlock();
        }
      }
    }
  }
}

std::optional<std::string> ReplServer::get_msg() {
  server_mutex.lock();
  std::optional<std::string> result;
  // Iterate through our client sockets, see if anyone has sent us a message
  // RACE - the first client wins
  for (auto it = client_sockets.begin(); it != client_sockets.end();) {
    auto req_bytes = it->get()->read_n(header_buffer.data(), header_buffer.size());
    if (req_bytes == -1) {
      // Disconnect
      lg::error("[nREPL:{}] Client Disconnected: {}", tcp_port, it->get()->address().to_string());
      it->get()->close();
      it = client_sockets.erase(it);
      continue;
    }
    if (req_bytes == header_buffer.size()) {
      auto* header = (ReplServerHeader*)(header_buffer.data());
      // get the body of the message
      int expected_size = header->length;
      req_bytes = it->get()->read_n(buffer.data(), expected_size);  // TODO - check error

      if (header->type == ReplServerMessageType::PING) {
        const auto success = ping_response(*it);
        if (!success) {
          it = client_sockets.erase(it);
        }
        server_mutex.unlock();
        return std::nullopt;
      } else if (header->type == ReplServerMessageType::EVAL) {
        std::string msg(buffer.data(), header->length);
        server_mutex.unlock();
        return std::make_optional(msg);
      }
    }
    it++;
  }
  server_mutex.unlock();
  return std::nullopt;
}
