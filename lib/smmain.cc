// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmain.hh"
#include "smfft.hh"
#include "smmath.hh"
#include <bse/bsemain.hh>
#include <bse/bseieee754.hh>
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

static bool sm_init_done_flag = false;

bool
sm_init_done()
{
  return sm_init_done_flag;
}

void
sm_init_plugin()
{
  assert (sm_init_done_flag == false);

  FFT::load_wisdom();
  int_sincos_init();
  sm_math_init();

  assert (bse_fpu_okround());
  assert (sm_round_positive (42.51) == 43);
  assert (sm_round_positive (3.14) == 3);
  assert (sm_round_positive (2.1) == 2);
  assert (sm_round_positive (0.7) == 1);
  assert (sm_round_positive (0.2) == 0);

  sm_init_done_flag = true;
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
