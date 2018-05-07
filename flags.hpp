// eve

#pragma once

#include <type_traits>

namespace eve {

template<typename e>
class flags {
public:
  using value_type = std::underlying_type_t<e>;

  constexpr flags(e v) : val(v) {}

  constexpr value_type operator*() const { return val; }

  constexpr bool active(e property) const {
    return val & static_cast<value_type>(property);
  }

  template<typename... f>
  constexpr bool all_of(f... args) const {
    return (... && active(args));
  }

  template<typename... f>
  constexpr bool any_of(f... args) const {
    return (... || active(args));
  }

private:
  value_type val;
};

template<typename f1, typename... f>
std::enable_if_t<std::is_enum<f1>::value, flags<f1>>
make_flags(f1 a1, f... args) {
  using type = std::underlying_type_t<f1>;
  return flags<f1>(static_cast<f1>(static_cast<type>(a1)
    & (... & static_cast<type>(args))));
}

template<typename e, typename v>
std::enable_if_t<std::is_integral<v>::value, flags<e>>
make_flags(v val) {
  static_assert(std::is_same<std::underlying_type_t<e>, v>::value);
  return static_cast<e>(val);
}

}

