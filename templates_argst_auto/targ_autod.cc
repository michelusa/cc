#include <iostream>
#include <tuple>
#include <string>

// help compiler deduct result type
template <typename T>
struct add_args {
  T result;
  template <typename... U>
  add_args(U... args) : result{(args + ...)} {}
};
template <typename... Ts>
add_args(Ts... args)->add_args<std::common_type_t<Ts...>>;

auto main() -> int {
  add_args numbers{1, 2, 3, 4, 10.1234};
  std::cout << numbers.result << std::endl;

  add_args some_strings{"abc", std::string("def")};
  std::cout << some_strings.result << std::endl;



  // compiler deduct argument type to tuple template
  std::tuple some_data{-5, "joe", "coffee", 14.45};
  auto [a, name, b, c] = some_data;
  std::cout << name << std::endl;
}
