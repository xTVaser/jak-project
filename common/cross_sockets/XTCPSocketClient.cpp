#include "XTCPSocketClient.h"

#include <string>

#include "common/log/log.h"
#include "common/repl/nrepl/ReplServer.h"

#include "third-party/fmt/core.h"

XTCPSocketClient::XTCPSocketClient(int _tcp_port) : tcp_port(_tcp_port) {}

XTCPSocketClient::XTCPSocketClient(std::string _ip, int _tcp_port) : ip(_ip), tcp_port(_tcp_port) {}

XTCPSocketClient::~XTCPSocketClient() {
  disconnect();
}

void XTCPSocketClient::disconnect() {
  if (tcp_conn) {
    tcp_conn.close();
  }
}

bool XTCPSocketClient::connect() {
  sockpp::initialize();
  // Open Socket
  if (!tcp_conn.connect(sockpp::inet_address(ip, tcp_port))) {
    lg::error("[XTCPSocketClient:{}] could not connect", tcp_port);
    return false;
  }

  lg::info("[XTCPSocketClient:{}] connected", tcp_port);

  return true;
}
