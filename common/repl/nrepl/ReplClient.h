#pragma once

#include "ReplServer.h"

#include "common/cross_sockets/XTCPSocketClient.h"

class ReplClient : public XTCPSocketClient {
 public:
  using XTCPSocketClient::XTCPSocketClient;
  virtual ~ReplClient() = default;

  ReplClient& operator=(const ReplClient&) { return *this; }

  // TODO - just void for now :(
  void eval(std::string form);
};
