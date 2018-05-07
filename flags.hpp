// eve

#pragma once

#include <type_traits>

namespace eve {

template<typename e>
class flags {
public:
  using value_type = std::underlying_type_t<e>;

  flags(e v) : val(v) {}

  value_type operator*() const { return val; }

  bool active(e property) const {
    return val & static_cast<value_type>(property);
  }

  template<typename... f>
  bool all_of(f... args) const {
    return (... && active(args));
  }

  template<typename... f>
  bool any_of(f... args) const {
    return (... || active(args));
  }

private:
  value_type val;
};

}

