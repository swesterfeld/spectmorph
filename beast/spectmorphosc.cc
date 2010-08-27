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

#include "spectmorphosc.genidl.hh"
#include "smaudio.hh"
#include "smlivedecoder.hh"
#include "smwavset.hh"
#include <bse/bsemathsignal.h>

using std::string;
using std::map;
using std::vector;

namespace SpectMorph {

using namespace Bse;

class AudioRepo {
  Birnet::Mutex mutex;
  map<string, WavSet *> wav_set_map;
public:
  WavSet *get_wav_set (const string& filename)
  {
    Birnet::AutoLocker lock (mutex);

    return wav_set_map[filename];
  }
  void put_wav_set (const string& filename, WavSet *wav_set)
  {
    Birnet::AutoLocker lock (mutex);

    wav_set_map[filename] = wav_set;
  }
} audio_repo;

class Osc : public OscBase {
  struct Properties : public OscProperties {
    Properties (Osc *osc) : OscProperties (osc)
    {
    }
  };
  class Module : public SynthesisModule {
  private:
    WavSet        *wav_set;
    LiveDecoder   *live_decoder;
    float          last_sync_level;
    float          current_freq;
    bool           need_retrigger;
    int            channel;
  public:
    Module() :
      wav_set (NULL),
      live_decoder (NULL),
      need_retrigger (false)
    {
      //
    }
    ~Module()
    {
      if (live_decoder)
        {
          delete live_decoder;
          live_decoder = NULL;
        }
    }
    void reset()
    {
      need_retrigger = true;
    }
    void process (unsigned int n_values)
    {
      //const gfloat *sync_in = istream (ICHANNEL_AUDIO_OUT).values;
      const gfloat *freq_in = istream (ICHANNEL_FREQ_IN).values;
      gfloat *audio_out = ostream (OCHANNEL_AUDIO_OUT).values;
      float new_freq = BSE_SIGNAL_TO_FREQ (freq_in[0]);
      if (need_retrigger)
        {
          retrigger (new_freq);
          need_retrigger = false;
        }
      live_decoder->process (n_values, NULL, NULL, audio_out);
    }
    void
    retrigger (float freq)
    {
      if (live_decoder)
        live_decoder->retrigger (channel, freq, mix_freq()); /* FIXME: channel should be module parameter */

      current_freq = freq;
    }
    void
    config (Properties *properties)
    {
      wav_set = audio_repo.get_wav_set (properties->filename.c_str());
      channel = properties->channel - 1;

      if (live_decoder)
        delete live_decoder;

      live_decoder = new LiveDecoder (wav_set);
    }
  };
public:
  void
  load_file (const string& filename)
  {
    BseErrorType error;

    WavSet *wav_set = new WavSet;
    error = wav_set->load (filename);
    if (!error)
      {
        audio_repo.put_wav_set (filename, wav_set);
      }
    else
      delete wav_set;
  }
  bool
  property_changed (OscPropertyID prop_id)
  {
    switch (prop_id)
      {
        case PROP_FILENAME:
          load_file (filename.c_str());
          break;
        default:
          break;
      }
    return false;
  }
  /* implement creation and config methods for synthesis Module */
  BSE_EFFECT_INTEGRATE_MODULE (Osc, Module, Properties);
};

BSE_CXX_DEFINE_EXPORTS();
BSE_CXX_REGISTER_EFFECT (Osc);

}
