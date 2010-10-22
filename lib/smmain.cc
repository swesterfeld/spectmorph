/* 
 * Copyright (C) 2010 Stefan Westerfeld
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

#include "smmain.hh"
#include "smfft.hh"
#include "smmath.hh"
#include <bse/bsemain.h>
#include <bse/bseieee754.h>
#include <stdio.h>
#include <assert.h>

namespace SpectMorph
{

float *int_sincos_table;

static bool use_sse = true;

void
sm_enable_sse (bool sse)
{
  use_sse = sse;
}

bool
sm_sse()
{
  return use_sse;
}

void
sm_init_plugin()
{
  FFT::load_wisdom();
  int_sincos_init();

  assert (bse_fpu_okround());
  assert (sm_round_positive (42.51) == 43);
  assert (sm_round_positive (3.14) == 3);
  assert (sm_round_positive (2.1) == 2);
  assert (sm_round_positive (0.7) == 1);
  assert (sm_round_positive (0.2) == 0);
}

void
sm_init (int *argc_p, char ***argv_p)
{
  SfiInitValue values[] = {
    { "stand-alone",            "true" }, /* no rcfiles etc. */
    { "wave-chunk-padding",     NULL, 1, },
    { "dcache-block-size",      NULL, 8192, },
    { "dcache-cache-memory",    NULL, 5 * 1024 * 1024, },
    { "load-core-plugins", "1" },
    { NULL }
  };
  bse_init_inprocess (argc_p, argv_p, NULL, values);
  sm_init_plugin();
}

}
