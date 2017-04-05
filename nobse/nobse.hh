// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_NO_BSE_HH
#define SPECTMORPH_NO_BSE_HH

#include <glib.h>
#include <string>
#include <vector>
#include <string>

/* Rapicorn fake */

#define RAPICORN_CLASS_NON_COPYABLE(Class)        private: Class (const Class&); Class& operator= (const Class&);
#define RAPICORN_PRINTF(format_idx, arg_idx)      __attribute__ ((__format__ (__printf__, format_idx, arg_idx)))

#define SPECTMORPH_NOBSE 1

/* --- macros for frequency valued signals --- */
double BSE_SIGNAL_TO_FREQ (double sig);

void bse_init_inprocess      (gint           *argc,
                              gchar         **argv,
                              const char     *app_name,
                              const std::vector<std::string>& args = std::vector<std::string>());

#endif
