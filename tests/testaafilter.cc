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
#include "smrandom.hh"
#include "smencoder.hh"
#include "smlivedecoder.hh"
#include "smfft.hh"

#include <bse/bsemathsignal.h>

#include <vector>
#include <assert.h>
#include <stdio.h>

using namespace SpectMorph;

using std::vector;
using std::max;
using std::min;

size_t
make_odd (size_t n)
{
  if (n & 1)
    return n;
  return n - 1;
}

void
encode_decode (vector<float>& audio_in, vector<float>& audio_out)
{
  EncoderParams enc_params;
  enc_params.mix_freq = 44100;
  enc_params.zeropad = 4;
  enc_params.frame_size_ms = 40;
  enc_params.frame_step_ms = enc_params.frame_size_ms / 4.0;
  enc_params.frame_size = make_odd (enc_params.mix_freq * 0.001 * enc_params.frame_size_ms);
  enc_params.frame_step = enc_params.mix_freq * 0.001 * enc_params.frame_step_ms;

  /* compute block size from frame size (smallest 2^k value >= frame_size) */
  uint64 block_size = 1;
  while (block_size < enc_params.frame_size)
    block_size *= 2;
  enc_params.block_size = block_size;
  vector<float> window (block_size);

  for (guint i = 0; i < window.size(); i++)
    {
      if (i < enc_params.frame_size)
        window[i] = bse_window_cos (2.0 * i / enc_params.frame_size - 1.0);
      else
        window[i] = 0;
    }

  Encoder encoder (enc_params);

  GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, enc_params.mix_freq, 440, audio_in.size(), &audio_in[0], NULL);
  BseErrorType error = gsl_data_handle_open (dhandle);
  assert (!error);

  const char *sm_file = "testaafilter.tmp.sm";
  encoder.encode (dhandle, 0, window, 1, false, true);
  encoder.save (sm_file, 440);

  WavSet wav_set;
  LiveDecoder decoder (&wav_set);

  WavSetWave new_wave;
  new_wave.midi_note = 60; // doesn't matter
  new_wave.channel = 0;
  new_wave.path = sm_file;
  wav_set.waves.push_back (new_wave);

  wav_set.save ("testaafilter.tmp.smset", true);

  wav_set = WavSet();
  error = wav_set.load ("testaafilter.tmp.smset");
  assert (!error);

  decoder.enable_noise (false);
  for (double freq = 10; freq < 70000; freq = min (freq * 1.1, freq + 10))
    {
      decoder.retrigger (0, freq, enc_params.mix_freq);
      decoder.process (audio_out.size(), 0, 0, &audio_out[0]);
      double peak = 0;
      for (size_t i = 0; i < audio_out.size(); i++)
        {
          peak = max (peak, fabs (audio_out[i]));
        }
      printf ("%f %.17g\n", freq, peak);
    }
}

int
main (int argc, char **argv)
{
  Random random;

  sm_init (&argc, &argv);

  vector<float> audio_in (44100);
  vector<float> audio_out (audio_in.size());

  size_t fade = audio_in.size() / 8;
  for (size_t i = 0; i < audio_in.size(); i++)
    {
      audio_in[i] = sin (i * 440 * 2 * M_PI / 44100);
      if (i < fade)
        audio_in[i] *= double (i) / fade;
      size_t ri = audio_in.size() - i;
      if (ri < fade)
        audio_in[i] *= double (ri) / fade;
    }

  encode_decode (audio_in, audio_out);

/*
  for (size_t i = 0; i < audio_out.size(); i++)
    printf ("%d %f %f\n", i, audio_in[i], audio_out[i]);
 */
 
  unlink ("testaafilter.tmp.sm");
  unlink ("testaafilter.tmp.smset");
}
