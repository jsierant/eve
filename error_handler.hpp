
#pragma once
#include "sys.hpp"
#include <stdexcept>
#include <string_view>

namespace eva {

class excetion_error_handler {
public:
  void sys_error(sys::error err, char const * sys_call_name) {
    throw std::runtime_error(std::string("System error, call: ")
      + sys_call_name + ",  msg: \'" + sys::to_string(err) + "\'");
  }
};

}
