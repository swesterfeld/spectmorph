// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smifftsynth.hh"
#include "smsinedecoder.hh"
#include "smlivedecoder.hh"
#include "smmath.hh"
#include "smmain.hh"
#include "smfft.hh"
#include "smutils.hh"
#include "smpandaresampler.hh"

#include <stdio.h>
#include <assert.h>

#include <vector>

using namespace SpectMorph;

using std::vector;
using std::max;
using std::min;

void
push_partial_f (AudioBlock& block, double freq_f, double mag_f, double phase_f)
{
  block.freqs.push_back (sm_freq2ifreq (freq_f));
  block.mags.push_back (sm_factor2idb (mag_f));
  block.phases.push_back (sm_bound<int> (0, sm_round_positive (phase_f / 2 / M_PI * 65536), 65535));
}

void
perf_test()
{
  const double mix_freq = 48000;
  const size_t block_size = 1024;

  vector<float> samples (block_size);
  vector<float> window (block_size);

  IFFTSynth synth (block_size, mix_freq, IFFTSynth::WIN_HANN);

  vector<double> freq_mag_phase;

  const double freq  = 440;
  const double mag   = 0.1;
  const double phase = 0.7;

  const double clocks_per_sec = 2500.0 * 1000 * 1000;

  freq_mag_phase.push_back (freq);
  freq_mag_phase.push_back (mag);
  freq_mag_phase.push_back (phase);

  int RUNS = 1000 * 1000 * 5;
  double start, end, t;

  synth.clear_partials();
  t = 1e30;
  for (int reps = 0; reps < 12; reps++)
    {
      start = get_time();
      for (int r = 0; r < RUNS; r++)
        synth.render_partial (freq_mag_phase[0], freq_mag_phase[1], freq_mag_phase[2]);
      end = get_time();
      t = min (t, end - start);
    }

  printf ("render_partial: clocks per sample: %f\n", clocks_per_sec * t / RUNS / block_size);

  AlignedArray<float, 16> sse_samples (block_size);

  synth.get_samples (&sse_samples[0]);  // first run may be slower

  RUNS = 100000;
  start = get_time();
  for (int r = 0; r < RUNS; r++)
    synth.get_samples (&sse_samples[0]);
  end = get_time();

  printf ("get_samples: clocks per sample: %f\n", clocks_per_sec * (end - start) / RUNS / block_size);

  SineDecoder sd (440, mix_freq, block_size, block_size / 2, SineDecoder::MODE_PHASE_SYNC_OVERLAP_IFFT);
  AudioBlock b, next_b;
  vector<double> dwindow (window.begin(), window.end());
  RUNS = 10000;

  int FREQS = 1000;
  for (int i = 0; i < FREQS; i++)
    push_partial_f (b, (440 + i) / 440.0, 0.9, 0.5);

  start = get_time();
  for (int r = 0; r < RUNS; r++)
    sd.process (b, next_b, dwindow, samples);
  end = get_time();
  printf ("SineDecoder (%d partials): clocks per sample: %f\n", FREQS, clocks_per_sec * (end - start) / FREQS / RUNS / block_size);

  SineDecoder sdo (440, mix_freq, block_size, block_size / 2, SineDecoder::MODE_PHASE_SYNC_OVERLAP);
  RUNS = 2000;

  start = get_time();
  for (int r = 0; r < RUNS; r++)
    sdo.process (b, next_b, dwindow, samples);
  end = get_time();
  printf ("Old SineDecoder (%d partials): clocks per sample: %f\n", FREQS, clocks_per_sec * (end - start) / FREQS / RUNS / block_size);
}

void
accuracy_test (double freq, double mag, double phase, double mix_freq, bool verbose,
               double *output_diff, double *frequency_diff)
{
  const size_t block_size = 1024;

  vector<float> spectrum (block_size);
  vector<float> samples (block_size);
  vector<float> window (block_size);

  for (size_t i = 0; i < window.size(); i++)
    window[i] = window_blackman_harris_92 (2.0 * i / block_size - 1.0);

  IFFTSynth synth (block_size, mix_freq, IFFTSynth::WIN_BLACKMAN_HARRIS_92);

  synth.clear_partials();
  synth.render_partial (freq, mag, phase);
  synth.get_samples (&samples[0]);

  VectorSinParams vsparams;
  vsparams.mix_freq = mix_freq;
  vsparams.freq = freq;
  vsparams.mag = mag;
  vsparams.phase = phase;
  vsparams.mode = VectorSinParams::REPLACE;

  AlignedArray<float, 16> aligned_decoded_sines (block_size);
  fast_vector_sinf (vsparams, &aligned_decoded_sines[0], &aligned_decoded_sines[block_size]);

  double max_diff = 0;
#if 0
  for (size_t i = 0; i < block_size; i++)
    {
      max_diff = max (max_diff, double (samples[i]) - aligned_decoded_sines[i] * window[i]);
      //printf ("%zd %.17g %.17g\n", i, samples[i], aligned_decoded_sines[i] * window[i]);
    }
  printf ("# max_diff(nq) = %.17g\n", max_diff);
#endif

  vsparams.freq = synth.quantized_freq (freq);
  fast_vector_sinf (vsparams, &aligned_decoded_sines[0], &aligned_decoded_sines[block_size]);

  synth.clear_partials();
  synth.render_partial (freq, mag, phase);
  synth.get_samples (&samples[0]);

  //printf ("# qfreq = %.17g\n", vsparams.freq);
  max_diff = 0;
  for (size_t i = 0; i < block_size; i++)
    {
      max_diff = max (max_diff, std::abs (double (samples[i]) - aligned_decoded_sines[i] * window[i]));
      //printf ("%zd %.17g %.17g\n", i, samples[i], aligned_decoded_sines[i] * window[i]);
    }
  if (verbose)
    printf ("%f %.17g\n", freq, max_diff);
  if (output_diff)
    *output_diff = max_diff;                       // output value diff
  if (frequency_diff)
    *frequency_diff = fabs (freq - vsparams.freq); // frequency diff due to quantization
}

void
test_negative_phase()
{
  const double mix_freq = 48000;
  const size_t block_size = 1024;

  IFFTSynth synth (block_size, mix_freq, IFFTSynth::WIN_BLACKMAN_HARRIS_92);

  vector<float> samples1 (block_size);
  vector<float> samples2 (block_size);

  synth.clear_partials();
  synth.render_partial (440, 1, 0.33);
  synth.get_samples (&samples1[0]);

  synth.clear_partials();
  synth.render_partial (440, 1, -2 * M_PI + 0.33);
  synth.get_samples (&samples2[0]);

  double max_diff = 0;
  for (size_t i = 0; i < block_size; i++)
    max_diff = max (max_diff, std::abs (double (samples1[i]) - samples2[i]));
  sm_printf ("# test_negative_phase: %.17g\n", max_diff);
  assert (max_diff < 1e-8);
}


void
test_accs()
{
  const double mix_freq = 48000;
  const size_t block_size = 1024;

  vector<double> window (block_size);
  vector<float> samples (block_size);
  vector<float> osamples (block_size);

  for (size_t i = 0; i < window.size(); i++)
    window[i] = window_cos (2.0 * i / block_size - 1.0);

  SineDecoder sd (440, mix_freq, block_size, block_size / 2, SineDecoder::MODE_PHASE_SYNC_OVERLAP_IFFT);
  AudioBlock b, next_b;

  push_partial_f (b, IFFTSynth (block_size, mix_freq, IFFTSynth::WIN_BLACKMAN_HARRIS_92).quantized_freq (440) / 440.0, 0.9, 0.5);

  sd.process (b, next_b, vector<double> (block_size) /* unused for ifft synth */, samples);

  SineDecoder sdo (440, mix_freq, block_size, block_size / 2, SineDecoder::MODE_PHASE_SYNC_OVERLAP);
  sdo.process (b, next_b, window, osamples);

  float max_diff = 0;
  for (size_t i = 0; i < block_size; i++)
    {
      printf ("%zd %.17g %.17g\n", i, samples[i], osamples[i]);
      max_diff = max (max_diff, fabs (samples[i] - osamples[i]));
    }
  printf ("# max_diff = %.17g\n", max_diff);
}

class ConstBlockSource : public LiveDecoderSource
{
  Audio      my_audio;
  AudioBlock my_audio_block;
public:
  ConstBlockSource (const AudioBlock& block, float mix_freq)
    : my_audio_block (block)
  {
    my_audio.frame_size_ms = 40;
    my_audio.frame_step_ms = 10;
    my_audio.attack_start_ms = 10;
    my_audio.attack_end_ms = 20;
    my_audio.zeropad = 4;
    my_audio.loop_type = Audio::LOOP_NONE;
    my_audio.mix_freq = mix_freq;

    if (my_audio_block.noise.empty())
      {
        my_audio_block.noise.resize (32); // all 0, no noise
      }
  }
  void retrigger (int channel, float freq, int midi_velocity)
  {
    my_audio.fundamental_freq = freq;
  }
  Audio *audio()
  {
    return &my_audio;
  }
  bool
  rt_audio_block (size_t index, RTAudioBlock& out_block)
  {
    out_block.assign (my_audio_block);
    return true;
  }
};

void
test_spect()
{
  double mix_freq = 48000;
  double freq = 440;
  const size_t block_size = 1024;

  AudioBlock audio_block;

  push_partial_f (audio_block, 1, 1, 0.9);

  ConstBlockSource source (audio_block, mix_freq);

  LiveDecoder live_decoder (&source, mix_freq);
  RTMemoryArea rt_memory_area;
  IFFTSynth synth (block_size, mix_freq, IFFTSynth::WIN_HANN);
  freq = synth.quantized_freq (freq);
  live_decoder.retrigger (0, freq, 127);

  vector<float> samples (block_size * 100);
  live_decoder.process (rt_memory_area, samples.size(), nullptr, &samples[0]);

  size_t power2 = 1;
  while (power2 * 2 < (samples.size() - block_size * 2))
    power2 *= 2;
  vector<float> wsig;
  for (size_t i = 0; i < power2; i++)
    wsig.push_back (window_blackman (2.0 * i / power2 - 1.0) * samples[block_size + i]);

  //for (size_t i = 0; i < power2; i++)
  //  printf ("%d %f\n", i, wsig[i]);

  float *spect = FFT::new_array_float (power2);
  FFT::fftar_float (power2, &wsig[0], spect);

  double max_mag = 0;
  for (size_t d = 0; d < power2; d += 2)
    {
      double re = spect[d];
      double im = spect[d+1];
      max_mag = max (sqrt (re * re + im * im), max_mag);
    }
  for (size_t d = 0; d < power2; d += 2)
    {
      double re = spect[d] / max_mag;
      double im = spect[d+1] / max_mag;
      printf ("%f %.17g\n", (d * mix_freq / 2.0) / power2, sqrt (re * re + im * im));
    }
  FFT::free_array_float (spect);
}

void
test_phase()
{
  const double mix_freq = 48000;
  const double freq = 440;

  AudioBlock audio_block;

  push_partial_f (audio_block, 1, 1, 0.9);

  ConstBlockSource source (audio_block, mix_freq);

  vector<float> samples (1024);

  RTMemoryArea rt_memory_area;
  LiveDecoder live_decoder (&source, mix_freq);
  live_decoder.enable_noise (false);
  live_decoder.retrigger (0, freq, 127);
  for (size_t block = 0; block < 10000000; block++)
    {
      live_decoder.process (rt_memory_area, samples.size(), nullptr, &samples[0]);

      float maxv = 0;
      for (size_t i = 0; i < samples.size(); i++)
        {
          maxv = max (samples[i], maxv);
        }
      if ((block % 997) == 0)
        printf ("%zd %.17g\n", block, maxv);
    }
}

void
test_portamento (bool expected)
{
  const double mix_freq = 48000;

  AudioBlock audio_block;

  push_partial_f (audio_block, 1, 1, 0.9);

  ConstBlockSource source (audio_block, mix_freq);


  RTMemoryArea rt_memory_area;
  LiveDecoder live_decoder (&source, mix_freq);
  live_decoder.enable_noise (false);

  double freq = 440;
  live_decoder.retrigger (0, freq, 127);

  vector<float> freq_in (5 * mix_freq);
  int porta_samples = mix_freq * 0.2;
  for (size_t i = 0; i < freq_in.size(); i++)
    {
      freq_in[i] = freq;
      if (i > mix_freq * 1.5 && i < mix_freq * 1.5 + porta_samples)
        freq *= pow (2, 1.0/porta_samples);
      if (i > mix_freq * 2.5 && i < mix_freq * 2.5 + porta_samples)
        freq /= pow (2, 1.0/porta_samples);
    }

  vector<float> samples (freq_in.size());
  if (expected) // expected result: sin wave
    {
      double phase = 0;
      for (size_t i = 0; i < samples.size(); i++)
        {
          samples[i] = sin (phase);
          phase += 2 * M_PI * freq_in[i] / 48000;
        }
    }
  else // actual result using live decoder
    {
      live_decoder.process (rt_memory_area, samples.size(), freq_in.data(), samples.data());
    }

  for (size_t i = 0; i < samples.size(); i++)
    {
      int dist_start = i;
      int dist_end = samples.size() - i;
      int dist = min (dist_start, dist_end);
      /* fade at the boundaries */
      double vol = min (dist / 4800., 1.);
      sm_printf ("%f\n", samples[i] * vol);
    }
}

void
test_portaslide (bool verbose)
{
  const double mix_freq = 48000;

  AudioBlock audio_block;

  push_partial_f (audio_block, 1, 1, 0.9);

  ConstBlockSource source (audio_block, mix_freq);

  RTMemoryArea rt_memory_area;
  LiveDecoder live_decoder (&source, mix_freq);
  live_decoder.enable_noise (false);

  const double test_freq = 120;
  double freq = test_freq;
  live_decoder.retrigger (0, freq, 127);

  vector<float> freq_in (5 * mix_freq);
  for (size_t i = 0; i < freq_in.size(); i++)
    {
      freq_in[i] = freq;
      if (i > mix_freq && freq < 20000)
        freq *= pow (2, 1. / 9000); //2.0 / mix_freq);
    }

  vector<float> samples (freq_in.size());
  live_decoder.process (rt_memory_area, samples.size(), freq_in.data(), samples.data());
#if 0
  double phase = 0;
  for (size_t i = 0; i < samples.size(); i++)
    {
      samples[i] = sin (phase);
      phase += 2 * M_PI * freq_in[i] / 48000;
    }
#endif

  // upsample factor 2
  using namespace PandaResampler;
  Resampler2 r2 (Resampler2::UP, 2, Resampler2::PREC_144DB);
  vector<float> up_samples (samples.size() * 2);
  r2.process_block (samples.data(), samples.size(), up_samples.data());

  auto pp_inter = PolyPhaseInter::the();

  vector<float> rsamples, rfreqs;
  double pos = 0;
  while (pos < samples.size())
    {
      int ipos = pos;
      rsamples.push_back (pp_inter->get_sample (up_samples, pos * 2));
      rfreqs.push_back (freq_in[ipos]);
      pos += test_freq / freq_in[ipos];
    }

  float db_side_low = -96, db_side_high = -96;
  float db_main_range_min = 96;
  float db_main_range_max = -96;
  for (float dest_freq = 110; dest_freq < 18000; dest_freq *= 1.01)
    {
      const size_t fft_size = 1024 * 16;
      size_t start = fft_size * 5;
      while (start + fft_size < rsamples.size() && rfreqs[start] < dest_freq)
        start++;

      double ww = 0;
      vector<float> wsig;
      for (size_t i = 0; i < fft_size; i++)
        {
          double win = window_blackman_harris_92 (2.0 * i / fft_size - 1.0);
          wsig.push_back (win * rsamples[start + i]);
          ww += win;
        }

#if 0
      for (size_t i = 0; i < fft_size; i++)
        sm_printf ("%zd %f\n", i, wsig[i]);
#endif

      float *spect = FFT::new_array_float (fft_size);
      FFT::fftar_float (fft_size, &wsig[0], spect);

      int main_lobe = 0;
      float db_max = -96;
      for (size_t d = 0; d < fft_size; d += 2)
        {
          double re = spect[d];
          double im = spect[d+1];
          double db = db_from_factor (sqrt (re * re + im * im) / ww * 2, -96);
          if (db > db_max)
            {
              main_lobe = d;
              db_max = db;
            }
        }
      float db_side = -96;
      for (size_t d = 0; d < fft_size; d += 2)
        {
          double re = spect[d];
          double im = spect[d+1];
          double db = db_from_factor (sqrt (re * re + im * im) / ww * 2, -96);
          int delta = std::abs (main_lobe - int (d)) / 2;
          if (delta > 4 && db > db_side) // blackman harris window has 9 bins main lobe width
            db_side = db;
        }
      if (verbose)
        sm_printf ("%f %f %f\n", dest_freq, db_max, db_side);
      if (dest_freq < 16000)
        {
          db_side_low = max (db_side_low, db_side);
        }
      else
        db_side_high = max (db_side_high, db_side);
      db_main_range_min = min (db_main_range_min, db_max);
      db_main_range_max = max (db_main_range_max, db_max);
#if 0
      for (size_t d = 0; d < fft_size; d += 2)
        {
          double re = spect[d];
          double im = spect[d+1];
          sm_printf ("%f %.17g\n", (d * mix_freq / 2.0) / fft_size, sqrt (re * re + im * im) / ww * 2);
        }
#endif
      FFT::free_array_float (spect);
    }
  sm_printf ("# Portamento: db_side_low   = %f\n", db_side_low);
  sm_printf ("# Portamento: db_side_high  = %f\n", db_side_high);
  sm_printf ("# Portamento: db_main_range = %f .. %f\n", db_main_range_min, db_main_range_max);
  assert (db_side_low < -71);
  assert (db_side_high < -55);
  assert (db_main_range_min > -0.19);
  assert (db_main_range_max < 0);
}

void
test_saw_perf()
{
  double mix_freq = 48000;
  double freq = 110;
  size_t PARTIALS = 100;
  const size_t block_size = 1024;

  AudioBlock audio_block;

  for (size_t partial = 1; partial <= PARTIALS; partial++)
    push_partial_f (audio_block, partial, 1.0 / partial, 0.9);

  vector<float> samples (block_size * 100);
  const int RUNS = 50;
  double t[2];
  for (int i = 0; i < 2; i++)
    {
      ConstBlockSource source (audio_block, mix_freq);

      RTMemoryArea rt_memory_area;
      LiveDecoder live_decoder (&source, mix_freq);
      live_decoder.enable_noise (false);
      live_decoder.enable_sines (i == 1);
      live_decoder.enable_debug_fft_perf (i == 0);
      live_decoder.precompute_tables (mix_freq);
      live_decoder.retrigger (0, freq, 127);

      double start, end;

      t[i] = 1e30;
      for (int reps = 0; reps < 12; reps++)
        {
          start = get_time();
          for (int r = 0; r < RUNS; r++)
            live_decoder.process (rt_memory_area, samples.size(), nullptr, &samples[0]);
          end = get_time();
          t[i] = min (t[i], end - start);
        }
    }

  const double clocks_per_sec = 2500.0 * 1000 * 1000;
  double time = t[1] - t[0]; // time without fft time
  printf ("LiveDecoder: clocks per sample per partial: %f\n", clocks_per_sec * time / RUNS / PARTIALS / samples.size());
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);
  FFT::debug_randomize_new_arrays (true); // catches uninitialized reads on newly allocated fft buffer

  if (argc == 2 && strcmp (argv[1], "perf") == 0)
    {
      perf_test();
      return 0;
    }
  if (argc == 2 && strcmp (argv[1], "saw_perf") == 0)
    {
      test_saw_perf();
      return 0;
    }
  if (argc == 2 && strcmp (argv[1], "accs") == 0)
    {
      test_accs();
      return 0;
    }
  if (argc == 2 && strcmp (argv[1], "spect") == 0)
    {
      test_spect();
      return 0;
    }
  if (argc == 2 && strcmp (argv[1], "phase") == 0)
    {
      test_phase();
      return 0;
    }
  if (argc == 2 && strcmp (argv[1], "portamento") == 0)
    {
      test_portamento (false);
      return 0;
    }
  if (argc == 2 && strcmp (argv[1], "portamento_sin") == 0)
    {
      test_portamento (true);
      return 0;
    }
  /*
   * generate a sine sweep over all frequencies to test portamento interpolation quality
   */
  if (argc == 2 && strcmp (argv[1], "portaslide") == 0)
    {
      test_portaslide (true);
      return 0;
    }
  const bool verbose = (argc == 2 && strcmp (argv[1], "verbose") == 0);
  const double mag = 0.991;
  const double phase = 0.5;
  double max_output_diff = 0;
  double max_freq_diff = 0;

  for (double freq = 20; freq < 24000; freq = min (freq * 1.01, freq + 2.5))
    {
      double output_diff = 1e32, freq_diff = 1e32;

      accuracy_test (freq, mag, phase, 48000, verbose, &output_diff, &freq_diff);

      max_output_diff = max (output_diff, max_output_diff);
      max_freq_diff = max (freq_diff, max_freq_diff);
    }
  printf ("# IFFTSynth: max_output_diff = %.17g\n", max_output_diff);
  printf ("# IFFTSynth: max_freq_diff = %.17g\n", max_freq_diff);
  assert (max_output_diff < 9e-5);
  assert (max_freq_diff < 0.1);

  test_portaslide (false);
  test_negative_phase();
}
