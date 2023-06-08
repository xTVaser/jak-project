#pragma once

// clang-format off
#if _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#endif
// clang-format on

namespace xsockets {
#ifdef __linux
const int TCP_SOCKET_LEVEL = SOL_TCP;
#elif _WIN32
const int TCP_SOCKET_LEVEL = IPPROTO_TCP;
#elif __APPLE__
const int TCP_SOCKET_LEVEL = IPPROTO_TCP;
#endif

bool socket_timed_out(int err) {
#ifdef OS_POSIX
  return err == EAGAIN;
#elif _WIN32
  return err == WSAETIMEDOUT;
#endif
}
}  // namespace xsockets
