// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_DEBUG_HH
#define SPECTMORPH_DEBUG_HH

#include <string>
#include <glib.h>

namespace SpectMorph
{

namespace Debug
{

void debug (const char *area, const char *fmt, ...) G_GNUC_PRINTF (2, 3);
void enable (const std::string& area);
void set_filename (const std::string& filename);

}

}

#endif
