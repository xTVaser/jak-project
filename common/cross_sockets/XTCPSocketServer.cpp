#include "XTCPSocketServer.h"

#include "common/common_types.h"
#include "common/log/log.h"

#include "third-party/fmt/core.h"

XTCPSocketServer::XTCPSocketServer(std::function<bool()> shutdown_callback,
                                   int _tcp_port,
                                   int _buffer_size)
    : want_exit_callback(std::move(shutdown_callback)) {
  tcp_port = _tcp_port;
  buffer.resize(_buffer_size);
}

XTCPSocketServer::~XTCPSocketServer() {
  close_server_socket();
}

void XTCPSocketServer::shutdown_server() {
  // Close the listening and accepted socket socket
  close_server_socket();
}

bool XTCPSocketServer::init_server() {
  sockpp::initialize();
  acceptor = sockpp::tcp_acceptor(tcp_port);
  if (!acceptor) {
    return false;
  }
  acceptor.set_non_blocking(true);

  server_initialized = true;
  lg::info("[XTCPSocketServer:{}] initialized", tcp_port);
  post_init();
  return true;
}

void XTCPSocketServer::close_server_socket() {
  if (acceptor) {
    acceptor.close();
  }
}
