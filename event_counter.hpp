// eve

#pragma once
#include <sys/eventfd.h>

#include <cstdint>
#include "sys.hpp"
#include "native_handle.hpp"
#include "error_handler.hpp"

namespace eve {

template <typename error_policy = excetion_error_handler>
class event_counter {
public:
  enum class property: int {
    non_blocking = EFD_NONBLOCK,
    semaphore = EFD_SEMAPHORE
  };
  friend property operator|(property l, property r) {
    return static_cast<property>(static_cast<int>(l)|static_cast<int>(r));
  }

  static auto const default_properties = property::non_blocking;

  using value_type = std::uint64_t;
  static value_type const default_init_value = 0;

  explicit event_counter(value_type init_value = default_init_value,
      property init_properties = default_properties)
    : handle(::eventfd(init_value, init_properties)) {
    if(handle == invalid_handle) {
      error.sys_error(sys::error(errno), "eventfd::create");
    }
  }

  event_counter(event_counter const&) = delete;
  event_counter& operator=(event_counter const&) = delete;
  event_counter(event_counter&&) = delete;
  event_counter& operator=(event_counter&&) = delete;

  ~event_counter() {
    sys::close(handle);
  }

  void notify(value_type inc_val = 1) {
    auto result = sys::write(handle, static_cast<std::byte*>(&inc_val),
                             sizeof(value_type));
    if(result.failed) {
      error.sys_error(result.err, "eventfd::write");
    }
  }

  value_type val() {
    value_type val = 0;
    auto result = sys::read(handle, static_cast<std::byte*>(&val),
                            sizeof(value_type));
    if(result.failed || result.size != sizeof(value_type)) {
      error.sys_error(result.err, "eventfd::read");
    }
    return 0;
  }

  auto native_handle() const { return handle; }

private:
  error_policy error;
  native_handle_t handle;
};
}

