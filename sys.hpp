// eve

#pragma once

#include "native_handle.hpp"

#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstddef>
#include <chrono>
#include <sys/epoll.h>
#include <sys/timerfd.h>

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
  auto result = ::write(fd.value(), buf, size);
  if(result < 0) {
    return { false, std::size_t(0), error(errno) };
  }
  return { true, std::size_t(result), error::no_error };
}

result read(native_handle_t fd, std::byte buf[], std::size_t size) {
  auto result = ::read(fd.value(), buf, size);
  if(result < 0) {
    return { false, std::size_t(0), error(errno) };
  }
  return { true, std::size_t(result), error::no_error };
}

bool close(native_handle_t fd) {
  return ::close(fd.value()) > 0;
}


namespace epoll {

std::optional<native_handle_t> create() {
  static int const simple_mode = 0;
  auto fd = native_handle_t{::epoll_create1(simple_mode)};
  if(fd == invalid_handle) { return {}; }
  return fd;
}

bool remove_handle(native_handle_t epoll, native_handle_t handle) {
  return ::epoll_ctl(epoll.value(),
    EPOLL_CTL_DEL, handle.value(), nullptr) != -1;
}

bool add_handle(native_handle_t epoll, native_handle_t handle,
                struct epoll_event const& event) {
  return ::epoll_ctl(epoll.value(), EPOLL_CTL_ADD, handle.value(),
                     const_cast<struct epoll_event*>(&event)) != -1;
}

result wait(native_handle_t epoll,
            struct epoll_event events[], std::size_t size) {
  static int const wait_indefinitely = -1;
  auto ready_events = ::epoll_wait(
      epoll.value(), events, size, wait_indefinitely);
  if(ready_events < 0) {
    return { false, std::size_t(0), error(errno) };
  }
  return { true, std::size_t(ready_events), error::no_error };
}

}

namespace timer {
struct result { bool failed; native_handle_t fd; error err; };
result create(clockid_t clock) {
  auto result = native_handle_t{::timerfd_create(clock, TFD_NONBLOCK)};
  if(result == invalid_handle) {
    return { true, result, error(errno) };
  }
  return { false, result, error::no_error };
}

timespec to_timespec(std::chrono::milliseconds duration) {
  using namespace std::chrono;
  auto sec_duration = duration_cast<seconds>(duration);
  auto remining = milliseconds(
    duration % duration_cast<milliseconds>(sec_duration));

  return { sec_duration.count(),
    duration_cast<nanoseconds>(remining).count() };
}

bool start(native_handle_t fd,
  std::chrono::milliseconds initial_expiration,
  std::chrono::milliseconds interval) {
  constexpr static int const flags = 0;

  struct itimerspec time{};
  time.it_value = to_timespec(initial_expiration);
  time.it_interval = to_timespec(interval);

  return ::timerfd_settime(fd.value(), flags, &time, nullptr) == 0;
}

bool start(native_handle_t fd,
  std::chrono::milliseconds initial_expiration) {
  constexpr static std::chrono::milliseconds const no_interval{0};
  return start(fd, initial_expiration, no_interval);
}

bool cancel(native_handle_t fd) {
  constexpr static std::chrono::milliseconds const no_initial_exp{0};
  return start(fd, no_initial_exp);
}

}

}
}

