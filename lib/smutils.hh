// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_UTIL_HH
#define SPECTMORPH_UTIL_HH

#include <string>
#include <vector>

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

#define SPECTMORPH_CLASS_NON_COPYABLE(Class) \
  Class (const Class&) = delete; \
  Class& operator= (const Class&) = delete

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
  INSTALL_DIR_INSTRUMENTS,
  INSTALL_DIR_FONTS
};

std::string sm_get_install_dir (InstallDir p);

// data directory is relocatable
void        sm_set_pkg_data_dir (const std::string& data_dir);

enum UserDir
{
  USER_DIR_INSTRUMENTS,
  USER_DIR_CACHE, /* FIXME: unify with sm_get_cache_dir */
  USER_DIR_DATA
};

std::string sm_get_user_dir (UserDir p);
std::string sm_get_default_plan();
std::string sm_get_cache_dir();

#ifdef SM_OS_MACOS
std::string sm_mac_documents_dir();
std::string sm_mac_application_support_dir();
#endif

enum DocumentsDir
{
  DOCUMENTS_DIR_INSTRUMENTS
};

std::string sm_get_documents_dir (DocumentsDir p);

class Error
{
public:
  enum class Code {
    NONE,
    FILE_NOT_FOUND,
    FORMAT_INVALID,
    PARSE_ERROR,
    STR
  };

  Error (Code code) :
    m_code (code)
  {
    switch (code)
      {
        case Code::NONE:
          m_message = "OK";
          break;

        case Code::FILE_NOT_FOUND:
          m_message = "No such file, device or directory";
          break;

        case Code::FORMAT_INVALID:
          m_message = "Invalid format";
          break;

        case Code::PARSE_ERROR:
          m_message = "Parsing error";
          break;

        default:
          m_message = "Unknown error";
      }
  }
  explicit
  Error (const std::string& message) :
    m_code (Code::STR),
    m_message (message)
  {
  }

  Code
  code()
  {
    return m_code;
  }
  const char *
  message()
  {
    return m_message.c_str();
  }
  operator bool()
  {
    return m_code != Code::NONE;
  }
private:
  Code        m_code;
  std::string m_message;
};

#ifdef SM_OS_WINDOWS
std::string sm_resolve_link (const std::string& link_file);
#endif

std::string sha1_hash (const unsigned char *data, size_t len);
std::string sha1_hash (const std::string& str);

double get_time();

std::string to_utf8 (const std::u32string& str);
std::u32string to_utf32 (const std::string& utf8);

Error read_dir (const std::string& dirname, std::vector<std::string>& files);
bool file_exists (const std::string& filename);
bool dir_exists (const std::string& dirname);

} // namespace SpectMorph

/* we want to be able to use sm_debug without extra includes */
#include "smdebug.hh"

#endif
