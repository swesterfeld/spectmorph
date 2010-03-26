/* 
 * Copyright (C) 2009-2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <bse/gsldatautils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

class MiniResampler
{
  GslDataHandle    *m_dhandle;
  GslDataPeekBuffer m_peek_buffer;
  double            m_speedup_factor;
public:
  MiniResampler (GslDataHandle *dhandle, double speedup_factor);

  int read (uint64 pos, size_t block_size, float *out);
  uint64 length();
};

MiniResampler::MiniResampler (GslDataHandle *dhandle, double speedup_factor)
{
  m_speedup_factor = speedup_factor;
  m_dhandle = dhandle;
  memset (&m_peek_buffer, 0, sizeof (m_peek_buffer));
  while (m_speedup_factor < 6)
    {
      m_dhandle = bse_data_handle_new_upsample2 (m_dhandle, 24);
      m_speedup_factor *= 2;
      BseErrorType error = gsl_data_handle_open (m_dhandle);
      if (error)
	{
	  fprintf (stderr, "foo\n");
	  exit (1);
	}
    }
}

int
MiniResampler::read (uint64 pos, size_t block_size, float *out)
{
  uint64 n_values = gsl_data_handle_n_values (m_dhandle);
  for (size_t i = 0; i < block_size; i++)
    {
      // linear interpolation
      double dpos = (pos + i) * m_speedup_factor;
      uint64 left_pos = dpos;
      uint64 right_pos = left_pos + 1;
      if (right_pos >= n_values)
	return (i - 1);                  // partial read
      double fade = dpos - left_pos;     // 0 => left sample 0.5 => mix both 1.0 => right sample
      out[i] = (1 - fade) * gsl_data_handle_peek_value (m_dhandle, left_pos, &m_peek_buffer)
	     + fade * gsl_data_handle_peek_value (m_dhandle, right_pos, &m_peek_buffer);
    }
  return block_size;
}

uint64
MiniResampler::length()
{
  return gsl_data_handle_n_values (m_dhandle) / m_speedup_factor;
}


