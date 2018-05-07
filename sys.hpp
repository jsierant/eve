// eva

#pragma once

#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstddef>
#include "native_handle.hpp"



namespace eva {
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


}
}

