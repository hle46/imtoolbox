#ifndef IMTOOLBOX_ULTILITY_H
#define IMTOOLBOX_ULTILITY_H

#include <iostream>
#include <sys/stat.h>
namespace imtoolbox {

// This is used to ignore compiler warning of unused variables
template <typename T> void ignore(const T &) {}

template <typename T>
inline typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
almost_equal(const T &x, const T &y, int ulp = 1) {
  // the machine epsilon has to be scaled to the magnitude of the values used
  // and multiplied by the desired precision in ULPs (units in the last place)
  return std::abs(x - y) <
             std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
         // unless the result is subnormal
         ||
         std::abs(x - y) < std::numeric_limits<T>::min();
}

enum class sort_t { none, descend, ascend };

#ifdef DEBUG
#ifdef ANDROID

// TODO: Equivalent functions to android

#else
namespace os_color {
constexpr const char *def = "\033[0m";
constexpr const char *red = "\033[1;31m";
const char *green = "\033[1;32m";
const char *yellow = "\033[1;33m";
const char *blue = "\033[1;34m";
const char *magenta = "\033[1;35m";
const char *cyan = "\033[1;36m";
const char *white = "\033[1;37m";
} // namespace os_color

inline void print_helper() {}

template <typename T> inline void print_helper(std::ostream &os, const T &t) {
  os << t;
}

template <typename T, typename... Args>
inline void print_helper(std::ostream &os, const T &t, const Args &... args) {
  std::cerr << t << ", ";
  print_helper(os, args...);
}

template <typename... Args>
inline void print_e(const Args &... args) {
  std::cerr << os_color::red;
  print_helper(std::cerr, args...);
  std::cerr << os_color::def;
}

template <typename... Args>
inline void println_e(const Args &... args) {
  std::cerr << os_color::red;
  print_helper(std::cerr, args...);
  std::cerr << os_color::def;
  std::cerr << "\n";
}

template <typename... Args>
inline void print_i(const Args &... args) {
  std::cout << os_color::yellow;
  print_helper(std::cout, args...);
  std::cout << os_color::def;
}

template <typename... Args>
inline void println_i(const Args &... args) {
  std::cout << os_color::yellow;
  print_helper(std::cout, args...);
  std::cout << os_color::def;
  std::cout << "\n";
}

bool exist(const char *file_name) {
  struct stat sb;
  return stat(file_name, &sb) == 0;
}
#endif // ANDROID
#else 
// TODO: Write to log file in release mode
#endif // DEBUG
} // namespace imtoolbox
#endif // IMTOOLBOX_ULTILITY_H
