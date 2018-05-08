// eve

#pragma once

#include "id.hpp"

namespace eve {

using native_handle_t = id<int, struct native_handle>;

constexpr static native_handle_t invalid_handle{-1};

}

