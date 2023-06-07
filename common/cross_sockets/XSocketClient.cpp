#include "XSocketClient.h"

#include <string>

#include "common/repl/nrepl/ReplServer.h"
#include <common/log/log.h>

#include "third-party/fmt/core.h"

XSocketClient::XSocketClient(int _tcp_port) {
  tcp_port = _tcp_port;
}

XSocketClient::~XSocketClient() {
  disconnect();
}

void XSocketClient::disconnect() {
  if (tcp_conn) {
    tcp_conn.close();
  }
}

bool XSocketClient::connect() {
  sockpp::initialize();
  // Open Socket
  if (!tcp_conn.connect(sockpp::inet_address("127.0.0.1", tcp_port))) {
    lg::error("[XSocketClient:{}] could not connect", tcp_port);
    return false;
  }

  lg::info("[XSocketClient:{}] connected", tcp_port);

  return true;
}
