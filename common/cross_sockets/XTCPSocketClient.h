#pragma once

#if _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif

#include <functional>
#include <mutex>
#include <thread>

#include "common/common_types.h"

#include "sockpp/tcp_connector.h"

/// @brief A cross platform generic socket client implementation
class XTCPSocketClient {
 public:
  XTCPSocketClient() = default;
  XTCPSocketClient(int _tcp_port);
  XTCPSocketClient(std::string _ip, int _tcp_port);
  ~XTCPSocketClient();

  bool connect();
  void disconnect();

  bool is_connected() { return connected; }

  std::string ip = "127.0.0.1";
  int tcp_port;
  sockpp::tcp_connector tcp_conn;
  bool connected = false;
};
