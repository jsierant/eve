// eve

#pragma once
#include "native_handle.hpp"
#include "event_counter.hpp"
#include "sys.hpp"

#include <sys/epoll.h>

#include <functional>
#include <unordered_map>

namespace eve {

template <typename error_policy = excetion_error_handler>
class reactor {
  static int const flags = 0;
  static int const wait_indefinitely = -1;
public:
  enum class event_type { read, write };
  using handler = std::function<void()>;

  reactor()
    : epollhandle(::epoll_create1(flags)),
      event(), events() {

    if(epollhandle == invalid_handle) {
      error.sys_error(sys::error(errno), "epoll_create1");
    }

    handlers[ctrlevent.native_handle()] =
      [this] () { stopped = true; ctrlevent.val(); };
    addEpollEvent(ctrlevent.native_handle(), event_type::read);
  }

  reactor(reactor const&) = delete;
  reactor& operator=(reactor const&) = delete;
  reactor(reactor&&) = delete;
  reactor& operator=(reactor&&) = delete;

  ~reactor() {
    sys::close(epollhandle);
  }

  template<typename resource>
  void register_handler(resource const& res, handler handler,
                        event_type type = event_type::read) {
    handlers[res.native_handle()] = handler;
    add_epoll_event(res.native_handle(), type);
  }

  template<typename resource>
  void remove_handler(resource const& res) {
    remove(res.native_handle());
  }

  void start() {
    while(!stopped) {
      auto ready_events = ::epoll_wait(epollhandle, events.data(),
                                       events.size(), wait_indefinitely);
      if (ready_events == -1) {
      }
//       for(auto const& event:
//           make_array_view(events.data(), ready_events)) {
//           handle(event.data.fd);
//       }
    }
  }

  void stop() {
    ctrlevent.trigger();
  }

private:
  void handle(native_handle_t handle) {
    auto handlerIt = handlers.find(handle);
    if (handlerIt == std::end(handlers)) {
    }
    handlerIt->second();
  }

  void remove(native_handle_t handle) {
    if(::epoll_ctl(epollhandle, EPOLL_CTL_DEL, handle, nullptr) == -1) {
    }
    handlers.erase(handle);
  }

  int convert_type(event_type t) {
    if(t == event_type::read) {
      return EPOLLIN;
    }
    return EPOLLOUT;
  }

  void add_epoll_event(native_handle_t handle, event_type type) {
    event.events = convert_type(type) | EPOLLRDHUP;
    event.data.fd = handle;
    if(::epoll_ctl(epollhandle, EPOLL_CTL_ADD, event.data.fd, &event) == -1) {
    }
  }

  error_policy error;
  bool stopped = false;
  event_counter<error_policy> ctrlevent;
  std::unordered_map<native_handle_t, handler> handlers;
  native_handle_t epollhandle;
  struct epoll_event event;
  std::array<struct epoll_event, 10> events;
};

}

