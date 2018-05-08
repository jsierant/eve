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

enum class event_type: int {
  read = EPOLLIN,
  write = EPOLLOUT,
  hangup = EPOLLHUP,
  connection_closed = EPOLLRDHUP,
  error = EPOLLERR
};

event_type operator|(event_type l, event_type r) {
  return static_cast<event_type>(static_cast<int>(l)|static_cast<int>(r));
}

template <typename error_policy = excetion_error_handler>
class reactor {
public:
  using handler = std::function<void(native_handle_t, flags<event_type>)>;

  reactor()
    : stopped(true), event(), events() {
    init_epoll();
  }

  reactor(reactor const&) = delete;
  reactor& operator=(reactor const&) = delete;
  reactor(reactor&&) = delete;
  reactor& operator=(reactor&&) = delete;

  ~reactor() {
    if(epoll) { sys::close(*epoll); }
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
    stopped = false;
    while(!stopped) {
      wait_and_handle_events();
    }
  }

  void stop() {
    ctl.trigger();
  }

private:
  void init_epoll() {
    epoll = sys::epoll::create();
    if(!epoll) {
      error.sys_error(sys::error(errno), "epoll::create");
      return;
    }
    register_ctl_handler();
  }

  void register_ctl_handler() {
    handlers[ctl.native_handle()] =
      [this] (auto, flags<event_type> type) {
        stopped = true;
        if(type.any_of(event_type::hangup, event_type::error)) {
          error.error("Reactor: unable to stop. Ctl event error.");
          return;
        }
        ctl.val();
      };
    add_epoll_event(ctl.native_handle(), event_type::read | event_type::error);
  }

  void wait_and_handle_events() {
    auto result = sys::epoll::wait(*epoll, events.data(), events.size());
    if (result.failed) {
      error.sys_critical(result.err, "epoll::wait");
    }
    for(std::size_t i = 0; i < result.size; ++i) {
      handle(events[i].data.fd, make_flags<event_type>(events[i].events));
    }
  }

  void handle(native_handle_t handle, event_type) {
    auto handlerIt = handlers.find(handle);
    if (handlerIt == std::end(handlers)) {
      error.warning("Reactor: undifiend handler for event! Removing handle.");
      remove(handle);
      return;
    }

    handlerIt->second();
  }

  void remove(native_handle_t handle) {
    if(!sys::epoll::remove_handle(*epoll, handle)) {
        error.critical("Reactor: unable to remove handle");
    }
    handlers.erase(handle);
  }

  bool add_epoll_event(native_handle_t handle, flags<event_type> type) {
    event.events = *type;
    event.data.fd = handle;
    if(!sys::epoll::add_handle(*epoll, handle, event)) {
      error.critical("Reactor: failed to add new handle");
      return false;
    }
    return true;
  }

  bool stopped;
  error_policy error;
  event_counter<error_policy> ctl;
  std::unordered_map<native_handle_t, handler> handlers;
  std::optional<native_handle_t> epoll;
  struct epoll_event event;
  std::array<struct epoll_event, 10> events;
};

}

