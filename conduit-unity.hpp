#include <experimental/coroutine>
#include <type_traits>
#include <cassert>
#include <type_traits>
namespace conduit {
template <class T>
T id(T const& x) {
  return x;
}
template <class T>
auto first(T&& xs) -> decltype(id(*xs.begin())) {
  return *xs.begin();
}
auto factory = [](auto sequence, auto...args) {
  return [=] {
    return sequence(args...);
  };
};
}
#include <memory>
namespace conduit {
template<class T>
struct promise_allocator {
  static void* alloc(std::size_t n) {
    return std::malloc(n);
  }
  static void dealloc(void* p) {
    std::free(p);
  }
};
}
using namespace std::experimental;
namespace conduit {
template<class T>
struct seq;
template <class T>
struct promise {
  T value;
  void* operator new(std::size_t n) {
    using A = promise_allocator<promise<T>>;
    return A::alloc(n);
  }
  void operator delete(void* p) {
    using A = promise_allocator<promise<T>>;
    A::dealloc((promise<T>*)p);
  }
  template <class U>
  auto yield_value(U&&value)
    -> decltype((this->value = std::forward<decltype(value)>(value)), suspend_always{}) {
    this->value = std::forward<decltype(value)>(value);
    return {};
  }
  constexpr suspend_always initial_suspend()const { return {}; }
  constexpr suspend_always final_suspend()const { return {}; }
  constexpr seq<T> get_return_object() noexcept {
    return {this};
  };
  void unhandled_exception() const { std::terminate(); }
  void return_void() const {}
};
template <class T>
struct iterator {
  coroutine_handle<promise<T>> handle;
  iterator& operator++() {
    assert(handle != nullptr);
    assert(!handle.done());
    if(handle)
      handle.resume();
    return *this;
  }
  T&& take() { return std::move(handle.promise().value); }
  constexpr T value() const noexcept { return handle.promise().value; }
  constexpr bool done() const noexcept { return !handle || handle.done(); }
  constexpr bool operator==(iterator const& rhs) const noexcept {
    return done() == rhs.done();
  }
  constexpr bool operator!=(iterator const& rhs) const noexcept {
    return done() != rhs.done();
  }
  T const& operator*() const { return handle.promise().value; }
  T& operator*() { return handle.promise().value; }
  T const* operator->() const { return &(operator*()); }
};
template <class T>
struct seq {
  using iterator_type = iterator<T>;
  using promise_type = promise<T>;
  using handle_type = coroutine_handle<promise_type>;
  iterator_type begin() {
    if (p) p.resume();
    return {p};
  }
  iterator_type end() { return {nullptr}; }
  seq(seq<T> const&) = delete;
  seq(seq<T>&& rhs)
    : p(rhs.p) {
      rhs.p = nullptr;
  }
  ~seq() {
    if (p) {
      p.destroy();
    }
  }
  T&& take() { return std::move(p.promise().value); }
  T const& get() const { return p.promise().value; }
  T& get() { return p.promise().value; }
  bool next() {
    assert(p != nullptr);
    assert(!p.done());
    p.resume();
    return !p.done();
  }
private:
  friend struct promise<T>;
  seq(promise_type* promise)
    : p{coroutine_handle<promise_type>::from_promise(*promise)}
  {}
  coroutine_handle<promise_type> p;
};
}
namespace conduit {
namespace F {
auto filter = [](auto xs, auto f) -> seq<decltype(first(xs))> {
  for (auto x : xs) {
    if (f(x))
      co_yield x;
  }
};
}
auto filter = [](auto&& f) {
  return [f](auto&& xs) {
    return F::filter(std::forward<decltype(xs)>(xs), f);
  };
};
}
namespace conduit {
namespace F {
auto scan = [](auto state, auto xs, auto f) -> seq<decltype(id(f(state, first(xs))))> {
  co_yield state;
  for (auto x : xs) {
    state = f(state, x);
    co_yield state;
  }
};
}
auto scan = [](auto&& initialState, auto&& f) {
  return [=](auto&& xs) {
    return F::scan(initialState, std::forward<decltype(xs)>(xs), f);
  };
};
}
namespace conduit {
namespace F {
template<class Xs>
auto concat(Xs&& xs) -> Xs&& {
  return std::forward<decltype(xs)>(xs);
}
template<class Xs, class Ys>
auto concat(Xs xs, Ys ys) -> seq<decltype(first(xs))> {
  for (auto x : xs) {
    co_yield x;
  }
    for (auto x : ys){
      co_yield x;
    }
}
}
auto concat = [](auto&&...xs) {
  return F::concat(std::forward<decltype(xs)>(xs)...);
};
}
namespace conduit {
auto endsWith = [](auto...xsf) {
  return [=](auto&& ys) {
    return F::concat(std::forward<decltype(ys)>(ys), xsf()...);
  };
};
}
namespace conduit {
template <class X, class F>
auto evaluate(X&&x, F&& f) {
  return f(std::forward<decltype(x)>(x));
}
template <class X, class F, class... Fs>
auto evaluate(X&&x, F&& f, Fs&&... fs) {
  return evaluate( f(std::forward<decltype(x)>(x)), std::forward<decltype(fs)>(fs)...);
}
template <class F, class... Fs>
auto compose(F f, Fs... fs) {
  return [=](auto&& x) {
    return evaluate(std::forward<decltype(x)>(x), f, fs...);
  };
}
namespace operators {
template <class Xs, class F>
auto operator >> (Xs&& xs, F&& f)
  -> decltype(xs.begin(), f(std::forward<decltype(xs)>(xs))) {
  return f(std::forward<decltype(xs)>(xs));
}
}
}
namespace conduit {
namespace F {
auto find = [](auto xs, auto f) -> seq<decltype(first(xs))> {
  for (auto x : xs) {
    if (f(x)) {
      co_yield x;
      break;
    }
  }
};
}
auto find = [](auto f) {
  return [f](auto&& xs) {
    return F::find(std::forward<decltype(xs)>(xs), f);
  };
};
}
#include <tuple>
namespace conduit {
namespace detail {
auto rangeUndone = [](auto&g) {
  auto& [_,b,e] = g;
  return b!=e;
};
auto all = [](auto...c) {
  return (c && ... && true);
};
template<class F, class...Xs>
auto zip(F f, Xs...xs) -> seq<decltype(f(*std::get<1>(xs)...))> {
  using namespace detail;
  while ( all(rangeUndone(xs)...) ) {
    co_yield f(*std::get<1>(xs)...);
    (++std::get<1>(xs) , ... , 0);
  }
}
}
namespace F {
template<class F, class...Xs>
auto zip(F&&f, Xs&&...xs) -> seq<decltype(id(f(first(xs)...)))> {
  return detail::zip(
      std::forward<decltype(f)>(f),
      std::tuple<
        decltype(id(xs)),
        decltype(id(xs.begin())),
        decltype(id(xs.end()))> {
          std::move(xs),
          xs.begin(),
          xs.end()
        }...
  );
};
}
auto zip = [](auto&&f, auto&&...xs) -> decltype( F::zip(std::forward<decltype(f)>(f), std::forward<decltype(xs)>(xs)...) ){
  return F::zip(std::forward<decltype(f)>(f), std::forward<decltype(xs)>(xs)...);
};
auto zipWith = [](auto g, auto f) {
  return [g, f](auto&& ys) {
    return zip(f, g(), std::forward<decltype(ys)>(ys));
  };
};
}
namespace conduit {
namespace F {
template<class Xs, class Xsf>
auto orElse(Xs xs, Xsf xsf) -> seq<decltype(first(xs))> {
  bool hasElements = 0;
  for (auto x: xs) {
    hasElements = true;
    co_yield x;
  }
  if (!hasElements) {
    for (auto x: xsf()) {
      co_yield x;
    }
  }
}
}
auto orElse = [](auto xsf) {
  return [xsf](auto&&xs) {
    return F::orElse(std::forward<decltype(xs)>(xs), xsf);
  };
};
}
#include <array>
#include <type_traits>
namespace conduit {
template<class...Xs>
auto just(Xs...xs) {
  if constexpr(sizeof...(xs)>0) {
    using T = std::common_type_t<decltype(xs)...>;
    return [=]() -> seq<T> {
      T data[] = {(T)xs...};
      for( auto x: data) {
        co_yield x;
      }
    };
  } else {
    return []() -> seq<int> {
      co_return;
    };
  }
}
}
namespace conduit {
namespace F {
template<class Xs>
auto take(Xs xs, unsigned n) -> seq<decltype(first(xs))> {
  if (!n) co_return;
  for (auto x: xs) {
    co_yield x;
    if (--n == 0)
      break;
  }
};
}
auto take = [](unsigned n) {
  return [n](auto&& xs) {
    return F::take(std::forward<decltype(xs)>(xs), n);
  };
};
}
#include <array>
#include <type_traits>
namespace conduit {
template<class...Xs>
auto cycle(Xs...xs) {
  if constexpr(sizeof...(xs)>0) {
    using T = std::common_type_t<decltype(xs)...>;
    return [=]() -> seq<T> {
      T data[] = {(T)xs...};
      while (1) {
        for( auto x: data) {
          co_yield x;
        }
      }
    };
  } else {
    return []() -> seq<int> {
      co_return;
    };
  }
}
}
namespace conduit {
namespace F {
auto dropUntil = [](auto xs, auto f) -> seq<decltype(id(first(xs)))> {
  auto it = xs.begin();
  auto e = xs.end();
  while (it != e) {
    auto x = *it;
    if (f(x)) {
      break;
    }
    ++it;
  }
  while (it != e) {
    co_yield *it;
    ++it;
  }
};
}
auto dropUntil = [](auto&& f) {
  return [f](auto&& xs) {
    return F::dropUntil(std::forward<decltype(xs)>(xs), f);
  };
};
}
namespace conduit {
namespace F {
auto takeWhile = [](auto xs, auto f) -> seq<decltype(first(xs))> {
  for (auto x: xs) {
    if (!f(x))
      break;
    co_yield x;
  }
};
}
auto takeWhile = [](auto&& f) {
  return [f](auto&& xs) {
    return F::takeWhile(std::forward<decltype(xs)>(xs), f);
  };
};
}
namespace conduit {
namespace detail {
template<class B, class E, class S>
auto range(B i, E e, S step) -> seq<B> {
  while (i < e) {
    co_yield i;
    i += step;
  }
};
template<class B, class E>
auto range(B i, E e) -> seq<B> {
  while (i < e) {
    co_yield i;
    ++i;
  }
};
template<class B = unsigned long>
auto range(B i = 0) -> seq<B> {
  while (true) {
    co_yield i;
    ++i;
  }
};
}
auto range = [](auto...args) -> decltype(
  detail::range(args...)
) {
  return detail::range(args...);
};
}
namespace conduit {
namespace F {
template<class Xs>
auto drop(Xs xs, unsigned n) -> seq<decltype(first(xs))> {
  for (auto x: xs) {
    if (n) {
      --n;
      continue;
    }
    co_yield x;
  }
};
}
auto drop = [](unsigned n) {
  return [n](auto&& xs) {
    return F::drop(std::forward<decltype(xs)>(xs), n);
  };
};
}
#include <vector>
namespace conduit {
auto toVector = [](std::size_t n=0) {
  return [n](auto&& xs) -> std::vector<decltype(first(xs))> {
    using T = decltype(first(xs));
    auto vec = std::vector<T>();
    vec.reserve(n);
    for (auto&&x: xs) {
      vec.emplace_back(std::move(x));
    }
    return vec;
  };
};
}
namespace conduit {
auto startsWith = [](auto...xsf) {
  return [=](auto&& ys) mutable {
    return F::concat(xsf()..., std::forward<decltype(ys)>(ys));
  };
};
}
namespace conduit {
namespace F {
auto flatMap = [](auto xs, auto f)
  -> seq<decltype(first(f(first(xs))))> {
  for (auto x : xs) {
    for(auto&& y: f(x)) {
      co_yield std::move(y);
    }
  }
};
}
auto flatMap = [](auto&& f) {
  return [f](auto&& xs) {
    return F::flatMap(std::forward<decltype(xs)>(xs), f);
  };
};
}
namespace conduit {
auto count = [](unsigned long start = 0) {
  return [start](auto xs) -> seq<unsigned long> {
    unsigned long i = start;
    for (auto x: xs) {
      (void)x;
      co_yield i;
      ++i;
    }
  };
};
}
namespace conduit {
namespace F {
template<class Xs, class Task>
auto forEach(Xs xs, Task task) -> seq<decltype(first(xs))> {
  for (auto x: xs) {
    task(x);
    co_yield x;
  }
};
}
auto forEach = [](auto task) {
  return [task](auto&&xs) -> seq<decltype(first(xs))> {
    return F::forEach(std::forward<decltype(xs)>(xs), task);
  };
};
}
namespace conduit {
static constexpr auto sum() {
  return [=](auto&& xs) {
    using T = decltype(first(xs));
    T result = T{};
    for (auto&&x: xs) {
      result+=x;
    }
    return result;
  };
};
}
namespace conduit {
namespace F {
auto map = [](auto xs, auto f) -> seq<decltype(id(f(first(xs))))> {
  for (auto x : xs) {
    co_yield f(x);
  }
};
}
auto map = [](auto f) {
  return [f](auto&& xs) {
    return F::map(std::forward<decltype(xs)>(xs), f);
  };
};
}
