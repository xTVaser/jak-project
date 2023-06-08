#pragma once

#include <functional>
#include <mutex>
#include <thread>

#include "common/common_types.h"

// clang-format off
#if _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include "sockpp/tcp_connector.h"
#endif
// clang-format on

/// @brief A cross platform generic socket client implementation
class XTCPSocketClient {
 public:
  XTCPSocketClient(int _tcp_port);
  ~XTCPSocketClient();

  XTCPSocketClient(const XTCPSocketClient&) = delete;
  XTCPSocketClient& operator=(const XTCPSocketClient&) = delete;

  bool connect();
  void disconnect();

  bool is_connected() { return connected; }

 protected:
  int tcp_port;
  sockpp::tcp_connector tcp_conn;
  bool connected = false;
};
