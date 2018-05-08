
#pragma once
#include "sys.hpp"
#include <stdexcept>
#include <string_view>

namespace eve {

class excetion_error_handler {
public:
  void sys_critical(sys::error err, char const * sys_call_name) {
    throw std::runtime_error(std::string("System error, call: ")
      + sys_call_name + ",  msg: \'" + sys::to_string(err) + "\'");
  }

  void critical(char const * msg) {
    throw std::runtime_error(msg);
  }

  void warning(char const *) {
  }
};

}
