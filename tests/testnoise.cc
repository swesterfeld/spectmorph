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

  const char *sm_file = "testnoise.tmp.sm";
  encoder.encode (dhandle, 0, window, 1, false, false);
  encoder.save (sm_file, 440);

  WavSet wav_set;
  LiveDecoder decoder (&wav_set);

  WavSetWave new_wave;
  new_wave.midi_note = 60; // doesn't matter
  new_wave.channel = 0;
  new_wave.path = sm_file;
  wav_set.waves.push_back (new_wave);

  wav_set.save ("testnoise.tmp.smset", true);

  wav_set = WavSet();
  error = wav_set.load ("testnoise.tmp.smset");
  assert (!error);

  float freq = 440;
  decoder.retrigger (0, freq, enc_params.mix_freq);
  decoder.process (audio_out.size(), 0, 0, &audio_out[0]);
}

void
avg_spectrum (const char *label, vector<float>& signal)
{
  const int block_size = 2048;
  int offset = 0;

  vector<float> avg (block_size / 2);
  vector<float> window (block_size);

  for (guint i = 0; i < window.size(); i++)
    window[i] = bse_window_cos (2.0 * i / block_size - 1.0);

  float *in = FFT::new_array_float (block_size);
  float *out = FFT::new_array_float (block_size);

  while (offset + block_size < signal.size())
    {
      std::copy (signal.begin() + offset, signal.begin() + offset + block_size, in);
      for (int i = 0; i < block_size; i++)
        in[i] *= window[i];

      FFT::fftar_float (block_size, in, out);
      for (int d = 0; d < block_size; d += 2)
        avg[d/2] += out[d] * out[d] + out[d+1] * out[d+1];
      offset += block_size / 4;
    }

  for (int i = 0; i < avg.size(); i++)
    printf ("%s %d %g\n", label, i, avg[i]);
}

int
main (int argc, char **argv)
{
  Random random;

  sm_init (&argc, &argv);

  vector<float> noise (44100 * 5);
  vector<float> audio_out (noise.size());

  for (size_t i = 0; i < noise.size(); i++)
    noise[i] = random.random_double_range (-0.5, 0.5);

  encode_decode (noise, audio_out);

  double e0 = 0, e1 = 0;
  for (size_t i = 0; i < noise.size(); i++)
    {
      e0 += noise[i] * noise[i];
      e1 += audio_out[i] * audio_out[i];
    }
  printf ("%f %f\n", e0, e1);

  avg_spectrum ("white-noise-in", noise);
  avg_spectrum ("white-noise-out", audio_out);

  unlink ("testnoise.tmp.sm");
  unlink ("testnoise.tmp.smset");
}
