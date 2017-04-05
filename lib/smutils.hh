// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_UTIL_HH
#define SPECTMORPH_UTIL_HH

#include <string>
#include <rapicorn.hh>

namespace SpectMorph
{

/* integer types */
typedef uint8_t  uint8;
typedef int64_t  int64;
typedef uint64_t uint64;

std::string string_printf (const char *format, ...) RAPICORN_PRINTF (1, 2);
std::string string_vprintf (const char *format, va_list vargs);

std::string string_locale_printf (const char *format, ...) RAPICORN_PRINTF (1, 2);

void        sm_printf (const char *format, ...) RAPICORN_PRINTF (1, 2);

enum InstallDir
{
  INSTALL_DIR_TEMPLATES
};

std::string sm_get_install_dir (InstallDir p);

enum UserDir
{
  USER_DIR_INSTRUMENTS
};

std::string sm_get_user_dir (UserDir p);
std::string sm_get_default_plan();

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

} // namespace SpectMorph

#endif
