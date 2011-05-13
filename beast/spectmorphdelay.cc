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

#include "spectmorphdelay.genidl.hh"
#include "smaudio.hh"
#include "smlivedecoder.hh"
#include "smwavset.hh"
#include "smmain.hh"
#include "smmorphplan.hh"
#include "smmorphplanvoice.hh"
#include "smmorphoutputmodule.hh"

#include <bse/bsemathsignal.h>
#include <bse/bseengine.h>

#include <stdio.h>

using std::string;
using std::map;
using std::vector;

namespace SpectMorph {

using namespace Bse;

class Delay : public DelayBase {
  struct Properties : public DelayProperties {
    Properties (Delay *osc) : DelayProperties (osc)
    {
    }
  };
  class Module : public SynthesisModule {
    struct ChannelState {
      vector<float> delay_buffer;
      size_t w, r;
    } cstate[3];
  public:
    Module()
    {
      for (size_t i = 0; i < 3; i++)
        {
          cstate[i].w = cstate[i].r = 0;
        }
    }
    void reset()
    {
      fill (cstate[0].delay_buffer.begin(), cstate[0].delay_buffer.end(), 1);
      fill (cstate[1].delay_buffer.begin(), cstate[1].delay_buffer.end(), 0);
      fill (cstate[2].delay_buffer.begin(), cstate[2].delay_buffer.end(), 0);
    }
    void
    process (unsigned int n_values, int ichannel, int ochannel, ChannelState& state)
    {
      if (istream (ichannel).connected && ostream (ochannel).connected)
        {
          const float *in = istream (ichannel).values;
          float *out = ostream (ochannel).values;
          for (size_t i = 0; i < n_values; i++)
            {
              state.delay_buffer[state.w++] = in[i];
              out[i] = state.delay_buffer[state.r++];
              if (state.w == state.delay_buffer.size())
                state.w = 0;
              if (state.r == state.delay_buffer.size())
                state.r = 0;
            }
        }
    }
    void
    process (unsigned int n_values)
    {
      process (n_values, ICHANNEL_GATE_IN, OCHANNEL_GATE_OUT, cstate[0]);
      process (n_values, ICHANNEL_AUDIO_IN1, OCHANNEL_AUDIO_OUT1, cstate[1]);
      process (n_values, ICHANNEL_AUDIO_IN2, OCHANNEL_AUDIO_OUT2, cstate[2]);
    }
    void
    config (Properties *properties)
    {
      float delay_ms = properties->delay_ms;
      for (size_t i = 0; i < 3; i++)
        {
          cstate[i].delay_buffer.resize (delay_ms * mix_freq() / 1000);
          cstate[i].w = cstate[i].delay_buffer.size() - 1;
          cstate[i].r = 0;
        }
    }
  };

public:
  Delay()
  {
    static bool sm_init_ok = false;
    if (!sm_init_ok)
      {
        // assume init by osc sm_init_plugin();
        sm_init_ok = true;
      }
  }
  ~Delay()
  {
  }
  /* implement creation and config methods for synthesis Module */
  BSE_EFFECT_INTEGRATE_MODULE (Delay, Module, Properties);
};

BSE_CXX_DEFINE_EXPORTS();
BSE_CXX_REGISTER_EFFECT (Delay);

}
