// eve

#include <functional>

namespace eve {
template<typename t, typename>
struct id {
  using value_type = t;

  constexpr explicit id(value_type v) : val(v) {}
  id() : val(0) {}

  bool operator==(id const& another) const {
    return val == another.val;
  }
  bool operator!=(id const& another) const {
    return !(*this == another);
  }

  bool operator<(id const& another) const {
    return val < another.val;
  }

  value_type value() const { return val; }

private:
  value_type val;
};

}

namespace std {
  template<typename t, typename unique>
  struct hash<eve::id<t, unique>> {
    auto operator()(eve::id<t, unique> const& id) const noexcept {
      return id.value();
    }
  };
}
