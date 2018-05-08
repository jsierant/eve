// eve

#pragma once

#include "sys.hpp"
#include <chrono>

namespace eve {

enum class clock: clockid_t {
  realtime = CLOCK_REALTIME,
  monotonic = CLOCK_MONOTONIC,
  boottime = CLOCK_BOOTTIME
};

namespace detail {

class timer {
public:
  explicit timer(std::chrono::milliseconds duration,
                 clock clk)
  : value(duration), started(false) {
    auto result = sys::timer::create(
      static_cast<std::underlying_type_t<clock>>(clk));
    if(result.failed) { }
    fd = result.fd;
  }

  ~timer() {
    sys::close(fd);
  }

  bool running() const noexcept {
    return started;
  }

  native_handle_t native_handle() const noexcept { return fd; }

protected:
  native_handle_t fd;
  std::chrono::milliseconds value;
  bool started;
};

}

class timer: private detail::timer {
public:
  explicit timer(std::chrono::milliseconds duration,
                 clock clk = clock::monotonic)
  : detail::timer(duration, clk) {}

  bool start() noexcept {
    sys::timer::start(fd, value);
    started = true;
  }

  void cancel() noexcept {
    sys::timer::cancel(fd);
    started = false;
  }

  std::chrono::milliseconds duration() const noexcept {
    return value;
  }
};

class periodic_timer: private detail::timer {
public:
  explicit periodic_timer(std::chrono::milliseconds interval,
                          clock clk = clock::monotonic)
  : detail::timer(interval, clk) {}

  bool start() noexcept {
    started = true;
    return sys::timer::start(fd, value, value);
  }

  bool start(std::chrono::milliseconds delay) noexcept {
    started = true;
    return sys::timer::start(fd, delay, value);
  }

  bool cancel() noexcept {
    started = false;
    return sys::timer::cancel(fd);
  }

  std::chrono::milliseconds interval() const noexcept {
    return value;
  }
};

}

