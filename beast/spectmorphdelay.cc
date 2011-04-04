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
    float delay_ms;
    vector<float> delay_buffer;
    size_t w, r;
  public:
    Module()
    {
      w = r = 0;
    }
    void reset()
    {
      fill (delay_buffer.begin(), delay_buffer.end(), 1);
    }
    void
    process (unsigned int n_values)
    {
      if (istream (ICHANNEL_GATE_IN).connected && ostream (OCHANNEL_GATE_OUT).connected)
        {
          const float *gate_in = istream (ICHANNEL_GATE_IN).values;
          float *gate_out = ostream (OCHANNEL_GATE_OUT).values;
          for (size_t i = 0; i < n_values; i++)
            {
              delay_buffer[w++] = gate_in[i];
              gate_out[i] = delay_buffer[r++];
              if (w == delay_buffer.size())
                w = 0;
              if (r == delay_buffer.size())
                r = 0;
            }
        }
    }
    void
    config (Properties *properties)
    {
      delay_ms = properties->delay_ms;
      delay_buffer.resize (delay_ms * mix_freq() / 1000);
      w = delay_buffer.size() - 1;
      r = 0;
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
