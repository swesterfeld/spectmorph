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
#include "smsinedecoder.hh"
#include "smnoisedecoder.hh"
#include <bse/bsemathsignal.h>

using std::string;
using std::map;
using std::vector;

namespace SpectMorph {

using namespace Bse;

class AudioRepo {
  Birnet::Mutex mutex;
  map<string, Audio *> audio_map;
public:
  Audio *get (const string& filename)
  {
    Birnet::AutoLocker lock (mutex);

    return audio_map[filename];
  }
  void put (const string& filename, Audio *audio)
  {
    Birnet::AutoLocker lock (mutex);

    audio_map[filename] = audio;
  }
} audio_repo;

class Osc : public OscBase {
  struct Properties : public OscProperties {
    Properties (Osc *osc) : OscProperties (osc)
    {
      // TODO
    }
  };
  class Module : public SynthesisModule {
  private:
    Audio *audio;
    SineDecoder *sine_decoder;
    NoiseDecoder *noise_decoder;
    size_t frame_size, frame_step;
    size_t have_samples;
    size_t pos;
    size_t env_pos;
    size_t frame_idx;
    float last_sync_level;
    vector<double> window;
    vector<double> samples;
  public:
    Module() :
      audio (NULL),
      sine_decoder (NULL),
      noise_decoder (NULL)
    {
      //
    }
    void reset()
    {
      frame_idx = 0;
      env_pos = 0;
      pos = 0;
      have_samples = 0;
    }
    void process (unsigned int n_values)
    {
      //const gfloat *sync_in = istream (ICHANNEL_AUDIO_OUT).values;
      gfloat *audio_out = ostream (OCHANNEL_AUDIO_OUT).values;

      for (unsigned int i = 0; i < n_values; i++)
        {
#if 0
          if (UNLIKELY (BSE_SIGNAL_RAISING_EDGE (last_sync_level, sync_in[i])))
            {
              frame_idx = 0;
              pos = 0;
              have_samples = 0;
              env_pos = 0;
              last_sync_level = sync_in[i];
            }
#endif
          if (have_samples == 0)
            {
              std::copy (samples.begin() + frame_step, samples.end(), samples.begin());
              std::fill (samples.begin() + frame_size - frame_step, samples.end(), 0);

              if ((frame_idx + 1) < audio->contents.size())
                {
                  Frame frame (audio->contents[frame_idx], frame_size);
                  Frame next_frame (audio->contents[frame_idx + 1], frame_size);

                  sine_decoder->process (frame, next_frame, window);
                  for (size_t i = 0; i < frame_size; i++)
                    samples[i] += frame.decoded_sines[i];

                  frame_idx++;
                }
              pos = 0;
              have_samples = frame_step;
            }

          g_assert (have_samples > 0);
          audio_out[i] = samples[pos];

          // decode envelope
          const double time_ms = env_pos * 1000.0 / mix_freq();
          if (time_ms < audio->attack_start_ms)
            {
              audio_out[i] = 0;
            }
          else if (time_ms < audio->attack_end_ms)
            {
              audio_out[i] *= (time_ms - audio->attack_start_ms) / (audio->attack_end_ms - audio->attack_start_ms);
            } // else envelope is 1

          pos++;
          env_pos++;
          have_samples--;
        }
    }
    void
    config (Properties *properties)
    {

      audio = audio_repo.get (properties->filename.c_str());
      printf ("cfg: %s %p\n", properties->filename.c_str(), audio);
      if (audio)
        {
          frame_size = audio->frame_size_ms * mix_freq() / 1000;
          frame_step = audio->frame_step_ms * mix_freq() / 1000;

          size_t block_size = 1;
          while (block_size < frame_size)
            block_size *= 2;

          window.resize (block_size);
          for (guint i = 0; i < window.size(); i++)
            {
              if (i < frame_size)
                window[i] = bse_window_cos (2.0 * i / frame_size - 1.0);
              else
                window[i] = 0;
            }

            if (noise_decoder)
              delete noise_decoder;
            noise_decoder = new NoiseDecoder (audio->mix_freq, mix_freq());

            if (sine_decoder)
              delete sine_decoder;

            SineDecoder::Mode mode = SineDecoder::MODE_PHASE_SYNC_OVERLAP;
            sine_decoder = new SineDecoder (mix_freq(), frame_size, frame_step, mode);

            samples.resize (frame_size);
            std::fill (samples.begin(), samples.end(), 0);
            have_samples = 0;
            pos = 0;
            frame_idx = 0;
            env_pos = 0;
          }
    }
  };
public:
  void
  load_file (const string& filename)
  {
    SpectMorph::Audio *audio = new SpectMorph::Audio;
    BseErrorType error = audio->load (filename);
    if (!error)
      audio_repo.put (filename, audio);
    else
      delete audio;
  }
  bool
  property_changed (OscPropertyID prop_id)
  {
    switch (prop_id)
      {
        case PROP_FILENAME:
          printf ("nfn: %s\n", filename.c_str());
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
