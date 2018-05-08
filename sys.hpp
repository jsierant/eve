// eve

#pragma once

#include "native_handle.hpp"

#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstddef>
#include <sys/epoll.h>

#include <optional>

namespace eve {
namespace sys {

enum class error: int {
  no_error = 0,
  would_block = EAGAIN,
  bad_file_desc = EBADF,
  fault = EFAULT,
  interrupted = EINTR,
  invalid = EINVAL,
  io = EIO,
  no_space_left = ENOSPC,
  permition = EPERM,
  pipe = EPIPE
};

static constexpr int invalid_result = -1;

auto to_string(error err) {
  return std::strerror(static_cast<int>(err));
}

struct result { bool failed; std::size_t size; error err; };

result write(native_handle_t fd, std::byte buf[], std::size_t size) {
  auto result = ::write(fd, buf, size);
  if(result < 0) {
    return { false, std::size_t(0), error(errno) };
  }
  return { true, std::size_t(result), error::no_error };
}

result read(native_handle_t fd, std::byte buf[], std::size_t size) {
  auto result = ::read(fd, buf, size);
  if(result < 0) {
    return { false, std::size_t(0), error(errno) };
  }
  return { true, std::size_t(result), error::no_error };
}

bool close(native_handle_t fd) {
  return ::close(fd) > 0;
}


namespace epoll {

std::optional<native_handle_t> create() {
  static int const simple_mode = 0;
  auto fd = ::epoll_create1(simple_mode);
  if(fd == invalid_handle) { return {}; }
  return fd;
}

bool remove_handle(native_handle_t epoll, native_handle_t handle) {
  return ::epoll_ctl(epoll, EPOLL_CTL_DEL, handle, nullptr) != -1;
}

bool add_handle(native_handle_t epoll, native_handle_t handle,
                struct epoll_event const& event) {
  return ::epoll_ctl(epoll, EPOLL_CTL_ADD, handle,
                     const_cast<struct epoll_event*>(&event)) != -1;
}

result wait(native_handle_t epoll,
            struct epoll_event events[], std::size_t size) {
  static int const wait_indefinitely = -1;
  auto ready_events = ::epoll_wait(
      epoll, events, size, wait_indefinitely);
  if(ready_events < 0) {
    return { false, std::size_t(0), error(errno) };
  }
  return { true, std::size_t(ready_events), error::no_error };
}


}

}
}

