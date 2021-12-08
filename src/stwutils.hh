// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_STW_UTILS_HH
#define SPECTMORPH_STW_UTILS_HH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

static bool
check_arg (uint         argc,
           char        *argv[],
           uint        *nth,
           const char  *opt,		  /* for example: --foo */
           const char **opt_arg = NULL)	  /* if foo needs an argument, pass a pointer to get the argument */
{
  g_return_val_if_fail (opt != NULL, false);
  g_return_val_if_fail (*nth < argc, false);

  const char *arg = argv[*nth];
  if (!arg)
    return false;

  uint opt_len = strlen (opt);
  if (strcmp (arg, opt) == 0)
    {
      if (opt_arg && *nth + 1 < argc)	  /* match foo option with argument: --foo bar */
        {
          argv[(*nth)++] = NULL;
          *opt_arg = argv[*nth];
          argv[*nth] = NULL;
          return true;
        }
      else if (!opt_arg)		  /* match foo option without argument: --foo */
        {
          argv[*nth] = NULL;
          return true;
        }
      /* fall through to error message */
    }
  else if (strncmp (arg, opt, opt_len) == 0 && arg[opt_len] == '=')
    {
      if (opt_arg)			  /* match foo option with argument: --foo=bar */
        {
          *opt_arg = arg + opt_len + 1;
          argv[*nth] = NULL;
          return true;
        }
      /* fall through to error message */
    }
  else
    return false;

  Options::print_usage();
  exit (1);
}

#endif
