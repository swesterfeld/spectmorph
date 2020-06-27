// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smutils.hh"
#include "config.h"

#include <string>
#include <locale>
#include <codecvt>

#include <assert.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <glib.h>

#ifdef SM_OS_MACOS
#include <xlocale.h>
#endif

#ifdef SM_OS_LINUX
#include <locale.h>
#include "smxdgdir.hh"
#endif

#ifndef SM_OS_WINDOWS
/********************************************************************************
* START: Code from Rapicorn
*********************************************************************************/

namespace {

// == locale ==

/// Class to push a specific locale_t for the scope of its lifetime.
class ScopedLocale {
  locale_t      locale_;
  /*copy*/      ScopedLocale (const ScopedLocale&) = delete;
  ScopedLocale& operator=    (const ScopedLocale&) = delete;
protected:
  explicit      ScopedLocale (locale_t scope_locale);
public:
  // explicit   ScopedLocale (const String &locale_name = ""); // not supported
  /*dtor*/     ~ScopedLocale ();
};

/// Class to push the POSIX/C locale_t (UTF-8) for the scope of its lifetime.
class ScopedPosixLocale : public ScopedLocale {
public:
  explicit        ScopedPosixLocale ();
  static locale_t posix_locale      (); ///< Retrieve the (UTF-8) POSIX/C locale_t.
};

ScopedLocale::ScopedLocale (locale_t scope_locale) :
  locale_ (NULL)
{
  if (!scope_locale)
    locale_ = uselocale (LC_GLOBAL_LOCALE);     // use process locale
  else
    locale_ = uselocale (scope_locale);         // use custom locale
  assert (locale_ != NULL);
}

ScopedLocale::~ScopedLocale ()
{
  uselocale (locale_);                          // restore locale
}

#if 0
ScopedLocale::ScopedLocale (const String &locale_name = "")
{
  /* this constructor should:
   * - uselocale (LC_GLOBAL_LOCALE) if locale_name == "",
   * - create newlocale from locale_name, use it and later delete it, but:
   * - freelocale(newlocale()) seems buggy on glibc-2.7 (crashes)
   */
}
#endif

ScopedPosixLocale::ScopedPosixLocale () :
  ScopedLocale (posix_locale())
{}

locale_t
ScopedPosixLocale::posix_locale ()
{
  static locale_t volatile posix_locale_ = NULL;
  if (!posix_locale_)
    {
      locale_t posix_locale = NULL;
      if (!posix_locale)
        posix_locale = newlocale (LC_ALL_MASK, "POSIX.UTF-8", NULL);
      if (!posix_locale)
        posix_locale = newlocale (LC_ALL_MASK, "C.UTF-8", NULL);
      if (!posix_locale)
        posix_locale = newlocale (LC_ALL_MASK, "POSIX", NULL);
      if (!posix_locale)
        posix_locale = newlocale (LC_ALL_MASK, "C", NULL);
      if (!posix_locale)
        posix_locale = newlocale (LC_ALL_MASK, NULL, NULL);
      assert (posix_locale != NULL);
      if (!__sync_bool_compare_and_swap (&posix_locale_, NULL, posix_locale))
        freelocale (posix_locale_);
    }
  return posix_locale_;
}

}

/********************************************************************************
* END: Code from Rapicorn
*********************************************************************************/
#else
struct ScopedPosixLocale /* dummy - no locale support under windows */
{
  ScopedPosixLocale() {}
};

#include "windows.h"
#include "shlobj.h"
#endif

using std::string;
using std::u32string;
using std::vector;

static string
string_current_vprintf (const char *format,
                        va_list     vargs)
{
  string s;
#ifndef SM_OS_WINDOWS
  char *str = NULL;
  if (vasprintf (&str, format, vargs) >= 0 && str)
    {
      s = str;
      free (str);
    }
  else
    s = format;
#else /* no vasprintf on windows */
  gchar *gstr = g_strdup_vprintf (format, vargs);
  s = gstr;
  g_free (gstr);
#endif
  return s;
}

namespace SpectMorph
{

string
string_vprintf (const char *format,
                va_list     vargs)
{
  ScopedPosixLocale posix_locale_scope; // pushes POSIX locale for this scope

  return string_current_vprintf (format, vargs);
}

string
string_printf (const char *format, ...)
{
  string str;
  va_list args;
  va_start (args, format);
  str = string_vprintf (format, args);
  va_end (args);
  return str;
}

string
string_locale_printf (const char *format, ...)
{
  string str;
  va_list args;
  va_start (args, format);
  str = string_current_vprintf (format, args);
  va_end (args);
  return str;
}

void
sm_printf (const char *format, ...)
{
  string str;
  va_list args;
  va_start (args, format);
  str = string_vprintf (format, args);
  va_end (args);

  printf ("%s", str.c_str());
}

static string pkg_data_dir = CONFIGURE_INSTALLPATH_PKGDATADIR;
static string bin_dir      = CONFIGURE_INSTALLPATH_BINDIR;

void
sm_set_pkg_data_dir (const string& data_dir)
{
  pkg_data_dir = data_dir;
}

void
sm_set_bin_dir (const string& dir)
{
  bin_dir = dir;
}

std::string
sm_get_install_dir (InstallDir p)
{
  switch (p)
    {
      case INSTALL_DIR_BIN:         return bin_dir;
      case INSTALL_DIR_TEMPLATES:   return pkg_data_dir + "/templates";
      case INSTALL_DIR_INSTRUMENTS: return pkg_data_dir + "/instruments";
      case INSTALL_DIR_FONTS:       return pkg_data_dir + "/fonts";
    }
  return "";
}

#ifdef SM_OS_WINDOWS
static string
dot_spectmorph_dir()
{
  /* Windows: use local app data dir */
  char buffer[MAX_PATH + 1];
  if (SHGetSpecialFolderPath (NULL, buffer, CSIDL_LOCAL_APPDATA, true))
    return string (buffer) + "/SpectMorph";
  else
    return "";
}

static string
win_documents_dir()
{
  char buffer[MAX_PATH + 1];
  if (SHGetSpecialFolderPath (NULL, buffer, CSIDL_MYDOCUMENTS, true))
    return buffer;
  else
    return "";
}

string
sm_resolve_link (const string& link_file)
{
  string      dest_path;
  IShellLink* psl;
  bool        com_need_uninit = false;
  HWND        hwnd = NULL;

  // Get a pointer to the IShellLink interface.
  HRESULT hres = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
  if (hres == CO_E_NOTINITIALIZED)
    {
      // COM was not initialized
      CoInitialize (NULL);
      com_need_uninit = true;

      hres = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    }

  if (SUCCEEDED (hres))
    {
      IPersistFile* ppf;

      // Get a pointer to the IPersistFile interface.
      hres = psl->QueryInterface (IID_IPersistFile, (void**)&ppf);

      if (SUCCEEDED (hres))
        {
          WCHAR wsz[MAX_PATH];

          // Ensure that the string is Unicode.
          MultiByteToWideChar (CP_ACP, 0, link_file.c_str(), -1, wsz, MAX_PATH);

          // Add code here to check return value from MultiByteWideChar
          // for success.

          // Load the shortcut.
          hres = ppf->Load (wsz, STGM_READ);

          if (SUCCEEDED (hres))
            {
              // Resolve the link.
              hres = psl->Resolve (hwnd, 0);

              if (SUCCEEDED (hres))
                {
                  // Get the path to the link target.
                  WIN32_FIND_DATA wfd;
                  char            szGotPath[MAX_PATH];

                  hres = psl->GetPath (szGotPath, MAX_PATH, &wfd, SLGP_SHORTPATH);
                  if (SUCCEEDED (hres))
                    {
                      dest_path = szGotPath;
                    }
                }
            }

          // Release the pointer to the IPersistFile interface.
          ppf->Release();
        }

      // Release the pointer to the IShellLink interface.
      psl->Release();
    }
  if (com_need_uninit)
    CoUninitialize();

  return dest_path;
}
#endif
#ifdef SM_OS_MACOS
static string
dot_spectmorph_dir()
{
  return sm_mac_application_support_dir() + "/SpectMorph";
}
#endif
#ifdef SM_OS_LINUX
static string
dot_spectmorph_dir()
{
  const char *home = g_get_home_dir();
  return home ? string (home) + "/.spectmorph" : "";
}
#endif

static bool
dir_exists (const string& dirname)
{
  struct stat st;

  if (stat (dirname.c_str(), &st) == 0)
    {
      return S_ISDIR (st.st_mode);
    }
  return false;
}

string
sm_get_user_dir (UserDir p)
{
  switch (p)
    {
      case USER_DIR_INSTRUMENTS: return dot_spectmorph_dir() + "/instruments";
      case USER_DIR_CACHE:       return dot_spectmorph_dir() + "/cache";
      case USER_DIR_DATA:        return dot_spectmorph_dir();
    }
  return "";
}

string
sm_get_documents_dir (DocumentsDir p)
{
#ifdef SM_OS_MACOS
  string documents = sm_mac_documents_dir(); // macOS -> "~/Documents/SpectMorph/Instruments/User"
#endif
#ifdef SM_OS_LINUX
  /*
   * we want to use XDG_DOCUMENTS_DIR to store our documents
   *
   * however, since old versions of SpectMorph (0.5.0, 0.5.1) used ~/SpectMorph
   * to store user defined instruments, we use this directory for backwards
   * compatibility if it exists
   */
  string documents = g_get_home_dir();          // Linux (backwards compat) -> "~/SpectMorph/Instruments/User"

  if (!dir_exists (documents + "/SpectMorph"))
    documents = xdg_dir_lookup ("DOCUMENTS");   // Linux (modern) -> "~/Documents/SpectMorph/Instruments/User"
#endif
#ifdef SM_OS_WINDOWS
  string documents = win_documents_dir();    // Windows -> "C:/Users/Stefan/Documents/SpectMorph/Instruments/User"
#endif
  switch (p)
    {
      case DOCUMENTS_DIR_INSTRUMENTS: return documents + "/SpectMorph/Instruments";
    }
  return "";
}

string
sm_get_cache_dir()   /* used by smenccache */
{
#ifdef SM_OS_LINUX
  const char *xdg_cache = getenv ("XDG_CACHE_HOME");
  if (xdg_cache && g_path_is_absolute (xdg_cache))
    return xdg_cache;

  const char *home = g_get_home_dir();
  assert (home);

  return string (home) + "/.cache";
#else
  return dot_spectmorph_dir() + "/cache";
#endif
}

std::string
sm_get_default_plan()
{
  return sm_get_install_dir (INSTALL_DIR_TEMPLATES) + "/2-instruments-linear-lfo.smplan";
}

string
sha1_hash (const unsigned char *data, size_t len)
{
  char *result = g_compute_checksum_for_data (G_CHECKSUM_SHA1, data, len);
  string hash = result;
  g_free (result);

  return hash;
}

string
sha1_hash (const string& str)
{
  return sha1_hash (reinterpret_cast<const unsigned char *> (str.data()), str.size());
}

double
get_time()
{
  /* return timestamp in seconds as double */
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

string
to_utf8 (const u32string& str)
{
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  return conv.to_bytes (str);
}

u32string
to_utf32 (const string& utf8)
{
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  return conv.from_bytes (utf8);
}

Error
read_dir (const string& dirname, vector<string>& files)
{
  GError *gerror = nullptr;
  const char *filename;

  GDir *dir = g_dir_open (dirname.c_str(), 0, &gerror);
  if (gerror)
    {
      Error error (gerror->message);
      g_error_free (gerror);
      return error;
    }
  files.clear();
  while ((filename = g_dir_read_name (dir)))
    files.push_back (filename);
  g_dir_close (dir);

  return Error::Code::NONE;
}

}
