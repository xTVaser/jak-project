#pragma once

#include <optional>
#include <set>

#include "common/cross_sockets/XTCPSocketServer.h"

enum ReplServerMessageType { PING = 0, EVAL = 10, SHUTDOWN = 20 };

struct ReplServerHeader {
  u32 length;
  u32 type;
};

class ReplServer : public XTCPSocketServer {
 public:
  using XTCPSocketServer::XTCPSocketServer;
  virtual ~ReplServer();

  void post_init() override;

  std::optional<std::string> get_msg();

 private:
  int max_clients = 100;
  std::vector<char> header_buffer = std::vector<char>((int)sizeof(ReplServerHeader));

  std::vector<std::unique_ptr<sockpp::tcp_socket>> client_sockets = {};
  bool kill_accept_thread = false;
  bool accept_thread_running = false;
  std::thread accept_thread;
  std::mutex server_mutex;

  void accept_thread_func();
  bool ping_response(std::unique_ptr<sockpp::tcp_socket>& socket);
};
