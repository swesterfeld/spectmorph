// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_NO_BSE_HH
#define SPECTMORPH_NO_BSE_HH

#include <glib.h>
#include <string>
#include <vector>
#include <string>

/* Rapicorn fake */


#define SPECTMORPH_NOBSE 1

/* --- macros for frequency valued signals --- */
double BSE_SIGNAL_TO_FREQ (double sig);

void bse_init_inprocess      (gint           *argc,
                              gchar         **argv,
                              const char     *app_name,
                              const std::vector<std::string>& args = std::vector<std::string>());

#endif
