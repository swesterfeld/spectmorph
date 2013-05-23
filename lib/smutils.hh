// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_UTIL_HH
#define SPECTMORPH_UTIL_HH

#include <string>
#include <rapicorn.hh>

namespace SpectMorph
{

using Rapicorn::AlignedArray;

std::string string_printf (const char *format, ...) RAPICORN_PRINTF (1, 2);
std::string string_locale_printf (const char *format, ...) RAPICORN_PRINTF (1, 2);
void        sm_printf (const char *format, ...) RAPICORN_PRINTF (1, 2);

} // namespace SpectMorph

#endif
