// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smutils.hh"

#include <string>

using std::string;

static string
string_vprintf (const char *format,
                va_list     vargs)
{
  char *str = NULL;
  if (vasprintf (&str, format, vargs) >= 0 && str)
    {
      string s = str;
      free (str);
      return s;
    }
  else
    return format;
}

namespace SpectMorph
{

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

}
