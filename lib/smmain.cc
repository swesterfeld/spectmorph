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

  sm_init_done_flag = true;
}

void
sm_init (int *argc_p, char ***argv_p)
{
  bse_init_inprocess (argc_p, *argv_p, NULL);
  sm_init_plugin();
}

}
