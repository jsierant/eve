// eve

#pragma once
#include "native_handle.hpp"
#include "event_counter.hpp"
#include "sys.hpp"
#include "flags.hpp"

#include <sys/epoll.h>

#include <functional>
#include <unordered_map>

namespace eve {

template <typename error_policy = excetion_error_handler>
class reactor {
  static int const simple_mode = 0;
  static int const wait_indefinitely = -1;
public:
  enum class event_type: int {
    read = EPOLLIN,
    write = EPOLLOUT,
    hungup = EPOLLHUP,
    connection_closed = EPOLLRDHUP,
    error = EPOLLERR
  };
  friend event_type operator|(event_type l, event_type r) {
    return static_cast<event_type>(static_cast<int>(l)|static_cast<int>(r));
  }

  using handler = std::function<void(native_handle_t, flags<event_type>)>;

  reactor()
    : epollhandle(::epoll_create1(simple_mode)),
      event(), events() {

    if(epollhandle == invalid_handle) {
      error.sys_error(sys::error(errno), "epoll_create1");
    }

    handlers[ctrlevent.native_handle()] =
      [this] (auto, flags<event_type> type) {
        stopped = true;
        if(type.any_of(event_type::hangup, event_type::error)) {
          error.error("Reactor: unable to stop. Ctl event error.");
          return;
        }
        ctrlevent.val();
      };
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
      if (ready_events < 0) {
        error.sys_error(sys::error(errno), "epoll_wait");
      }
      for(std::size_t i = 0; i < static_cast<std::size_t>(ready_events); ++i) {
        handle(events[i].data.fd, make_flags<event_type>(events[i].events));
      }
    }
  }

  void stop() {
    ctrlevent.trigger();
  }

private:
  void handle(native_handle_t handle, event_type) {
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

  void add_epoll_event(native_handle_t handle, flags<event_type> type) {
    event.events = *type;
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

