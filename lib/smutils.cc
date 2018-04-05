// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smutils.hh"
#include "config.h"

#include <string>
#include <assert.h>
#include <stdarg.h>
#include <glib.h>

#ifdef SM_OS_MACOS
#include <xlocale.h>
#endif

#ifdef SM_OS_LINUX
#include <locale.h>
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

void
sm_set_pkg_data_dir (const string& data_dir)
{
  pkg_data_dir = data_dir;
}

std::string
sm_get_install_dir (InstallDir p)
{
  switch (p)
    {
      case INSTALL_DIR_TEMPLATES:   return pkg_data_dir + "/templates";
      case INSTALL_DIR_INSTRUMENTS: return pkg_data_dir + "/instruments";
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
#else
static string
dot_spectmorph_dir()
{
  /* Linux/macOS */
  const char *home = g_get_home_dir();
  return home ? string (home) + "/.spectmorph" : "";
}
#endif

std::string
sm_get_user_dir (UserDir p)
{
  switch (p)
    {
      case USER_DIR_INSTRUMENTS: return dot_spectmorph_dir() + "/instruments";
      case USER_DIR_DATA:        return dot_spectmorph_dir();
    }
  return "";
}

std::string
sm_get_default_plan()
{
  return sm_get_install_dir (INSTALL_DIR_TEMPLATES) + "/2-instruments-linear-lfo.smplan";
}

const char *
sm_error_blurb (Error error)
{
  /* FIXME: may need translation */
  switch (error)
    {
      case Error::NONE:            return "OK";
      case Error::FILE_NOT_FOUND:  return "No such file, device or directory";
      case Error::FORMAT_INVALID:  return "Invalid format";
      case Error::PARSE_ERROR:     return "Parsing error";
    }
  return "Unknown error";
}

}
