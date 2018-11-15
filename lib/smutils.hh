// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_UTIL_HH
#define SPECTMORPH_UTIL_HH

#include <string>

// operating system: one of these three
#if WIN32
  #define SM_OS_WINDOWS
#elif __APPLE__
  #define SM_OS_MACOS
#elif __linux__
  #define SM_OS_LINUX
#else
  #error "unsupported platform"
#endif

// detect compiler
#if __clang__
  #define SM_COMP_CLANG
#elif __GNUC__ > 2
  #define SM_COMP_GCC
#else
  #error "unsupported compiler"
#endif

namespace SpectMorph
{

/* integer types */
typedef uint8_t       uint8;
typedef uint32_t      uint32;
typedef int64_t       int64;
typedef uint64_t      uint64;
typedef unsigned int  uint;

#define SPECTMORPH_CLASS_NON_COPYABLE(Class)        private: Class (const Class&); Class& operator= (const Class&);

#ifdef SM_COMP_GCC
  #define SPECTMORPH_PRINTF(format_idx, arg_idx)      __attribute__ ((__format__ (gnu_printf, format_idx, arg_idx)))
#else
  #define SPECTMORPH_PRINTF(format_idx, arg_idx)      __attribute__ ((__format__ (__printf__, format_idx, arg_idx)))
#endif

std::string string_printf (const char *format, ...) SPECTMORPH_PRINTF (1, 2);
std::string string_vprintf (const char *format, va_list vargs);

std::string string_locale_printf (const char *format, ...) SPECTMORPH_PRINTF (1, 2);

void        sm_printf (const char *format, ...) SPECTMORPH_PRINTF (1, 2);

enum InstallDir
{
  INSTALL_DIR_BIN,
  INSTALL_DIR_TEMPLATES,
  INSTALL_DIR_INSTRUMENTS
};

std::string sm_get_install_dir (InstallDir p);

// data directory is relocatable
void        sm_set_pkg_data_dir (const std::string& data_dir);

enum UserDir
{
  USER_DIR_INSTRUMENTS,
  USER_DIR_DATA
};

std::string sm_get_user_dir (UserDir p);
std::string sm_get_default_plan();
std::string sm_get_cache_dir();

enum class Error
{
  NONE = 0,
  FILE_NOT_FOUND,
  FORMAT_INVALID,
  PARSE_ERROR,
};

// convenience: provide comparision: (error == 0), (error != 0)
bool constexpr operator== (Error v, int64_t n) { return int64_t (v) == n; }
bool constexpr operator== (int64_t n, Error v) { return n == int64_t (v); }
bool constexpr operator!= (Error v, int64_t n) { return int64_t (v) != n; }
bool constexpr operator!= (int64_t n, Error v) { return n != int64_t (v); }

const char *sm_error_blurb (Error error);

#ifdef SM_OS_WINDOWS
std::string sm_resolve_link (const std::string& link_file);
#endif

std::string sha1_hash (const unsigned char *data, size_t len);
std::string sha1_hash (const std::string& str);

} // namespace SpectMorph

/* we want to be able to use sm_debug without extra includes */
#include "smdebug.hh"

#endif
