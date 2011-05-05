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

#include <vector>
#include <stdio.h>
#include "smaudio.hh"
#include "smwavset.hh"
#include "smlivedecoder.hh"
#include "smmain.hh"
#include "sminfile.hh"
#include <assert.h>
#include <bse/bsemathsignal.h>
#include <bse/gslfft.h>

using namespace SpectMorph;
using std::vector;
using std::min;
using std::max;
using std::string;
using std::set;

double
vector_delta (const vector<double>& a, const vector<double>& b)
{
  assert (a.size() == b.size());

  double d = 0;
  for (size_t i = 0; i < a.size(); i++)
    d += (a[i] - b[i]) * (a[i] - b[i]);
  return d;
}

static void
reconstruct (AudioBlock&     audio_block,
             vector<double>& signal,
             double          mix_freq)
{
  for (size_t partial = 0; partial < audio_block.freqs.size(); partial++)
    {
      double f     = audio_block.freqs[partial];
      double mag   = audio_block.mags[partial];
      double phase = audio_block.phases[partial];

      // do a phase optimal reconstruction of that partial
      for (size_t n = 0; n < signal.size(); n++)
        {
          signal[n] += sin (phase) * mag;
          phase += f / mix_freq * 2.0 * M_PI;
        }
    }
}

float
mag (float re, float im)
{
  return sqrt (re * re + im * im);
}

struct Attack
{
  double attack_start_ms;
  double attack_end_ms;
};

double
attack_error (const SpectMorph::Audio& audio, const vector< vector<double> >& unscaled_signal, const Attack& attack, vector<double>& out_scale)
{
  const size_t frames = unscaled_signal.size();
  double total_error = 0;

  for (size_t f = 0; f < frames; f++)
    {
      const vector<double>& frame_signal = unscaled_signal[f];
      size_t zero_values = 0;
      double scale = 1.0; /* init to get rid of gcc compiler warning */

      for (size_t n = 0; n < frame_signal.size(); n++)
        {
          const double n_ms = f * audio.frame_step_ms + n * 1000.0 / audio.mix_freq;
          double env;
          scale = (zero_values > 0) ? frame_signal.size() / double (frame_signal.size() - zero_values) : 1.0;
          if (n_ms < attack.attack_start_ms)
            {
              env = 0;
              zero_values++;
            }
          else if (n_ms < attack.attack_end_ms)  // during attack
            {
              const double attack_len_ms = attack.attack_end_ms - attack.attack_start_ms;

              env = (n_ms - attack.attack_start_ms) / attack_len_ms;
            }
          else // after attack
            {
              env = 1.0;
            }
          const double value = frame_signal[n] * scale * env;
          const double error = value - audio.contents[f].debug_samples[n];
          total_error += error * error;
        }
      out_scale[f] = scale;
    }
  return total_error;
}

void
check_usage (int argc, int need_argc, const string& usage)
{
  if (argc != need_argc)
    {
      printf ("usage: smtool <sm_file> %s\n", usage.c_str());
      exit (1);
    }
}

void
load_or_die (Audio& audio, const string& filename, const string& mode)
{
  AudioLoadOptions load_options = AUDIO_LOAD_DEBUG;

  if (mode == "fundamental-freq" || mode == "freq"
  ||  mode == "frameparams" || mode == "noiseparams" || mode == "attack" ||
      mode == "zero-values-at-start" || mode == "mix-freq")
    load_options = AUDIO_SKIP_DEBUG;

  BseErrorType error = audio.load (filename, load_options);
  if (error)
    {
      fprintf (stderr, "can't load file: %s\n",filename.c_str());
      exit (1);
    }
}

static bool
find_nan (vector<float>& data)
{
  for (size_t x = 0; x < data.size(); x++)
    if (isnan (data[x]))
      return true;
  return false;
}

static double
compute_energy (const Audio& audio, double percent, bool from_loop)
{
  double percent_start = percent - 5;
  double percent_stop = percent + 5;
  if (percent_start < 0 || percent_stop > 100)
    {
      fprintf (stderr, "bad volume percentage: %f\n", percent);
      exit (1);
    }

  Audio *noloop_audio = audio.clone();
  if (!from_loop)
    {
      noloop_audio->loop_type = Audio::LOOP_NONE;  // don't use looped signal, but original signal
    }

  WavSet smset;
  WavSetWave new_wave;
  new_wave.midi_note = 60; // doesn't matter
  new_wave.channel = 0;
  new_wave.velocity_range_min = 0;
  new_wave.velocity_range_max = 127;
  new_wave.audio = noloop_audio;
  smset.waves.push_back (new_wave);

  LiveDecoder decoder (&smset);
  // we need reproducable noise to get the same energy every time
  decoder.set_noise_seed (42);
  decoder.retrigger (0, audio.fundamental_freq, 127, audio.mix_freq);
  vector<float> samples;
  if (from_loop)
    {
      // at least one second, or twice the original len, whatever is longer
      samples.resize (audio.sample_count + MAX (audio.sample_count, audio.mix_freq));
    }
  else
    {
      samples.resize (audio.sample_count);
    }
  decoder.process (samples.size(), 0, 0, &samples[0]);

  double energy = 0;
  size_t energy_norm = 0;
  if (from_loop)
    {
      // start evaluating energy after end of original data (so we're counting the looped part only
      for (size_t pos = audio.sample_count; pos < samples.size(); pos++)
        {
          energy += samples[pos] * samples[pos];
          energy_norm++;
        }
    }
  else
    {
      for (size_t pos = 0; pos < samples.size(); pos++)
        {
          double percent = (pos * 100.0) / samples.size();
          if (percent > percent_start && percent < percent_stop)
            {
              energy += samples[pos] * samples[pos];
              energy_norm++;
            }
        }
    }
  return energy / energy_norm;
}

class Command
{
  string m_mode;
  bool   m_need_save;
public:
  static vector<Command *> *registry();
  Command (const string& mode)
  {
    registry()->push_back (this);
    m_mode = mode;
    m_need_save = false;
  }
  virtual bool
  parse_args (vector<string>& args)
  {
    return args.size() == 0;
  }
  virtual bool exec (Audio& audio) = 0;
  virtual void usage (bool one_line)
  {
    printf ("\n");
  }
  virtual ~Command()
  {
  }
  string mode()
  {
    return m_mode;
  }
  bool
  need_save()
  {
    return m_need_save;
  }
  void
  set_need_save (bool s)
  {
    m_need_save = s;
  }
};

vector<Command *> *
Command::registry()
{
  static vector<Command *> *rx = 0;
  if (!rx)
    rx = new vector<Command *>;
  return rx;
}

class VolumeCommand : public Command
{
  double percent;
public:
  VolumeCommand() : Command ("volume")
  {
  }
  bool
  parse_args (vector<string>& args)
  {
    if (args.size() == 1)
      {
        percent = atof (args[0].c_str());
        return true;
      }
    return false;
  }
  bool
  exec (Audio& audio)
  {
    const double energy = compute_energy (audio, percent, false);
    printf ("avg_energy: %.17g\n", energy);
    return true;
  }
  void
  usage (bool one_line)
  {
    printf ("<percent>\n");
  }
} volume_command;

class FundamentalFreqCommand : public Command
{
public:
  FundamentalFreqCommand() : Command ("fundamental-freq")
  {
  }
  bool
  exec (Audio& audio)
  {
    printf ("fundamental-freq: %f\n", audio.fundamental_freq);
    return true;
  }
} fundamental_freq_command;

class MixFreqCommand : public Command
{
public:
  MixFreqCommand() : Command ("mix-freq")
  {
  }
  bool
  exec (Audio& audio)
  {
    printf ("mix-freq: %f\n", audio.mix_freq);
    return true;
  }
} mix_freq_command;

class ZeroValuesAtStartCommand : public Command
{
public:
  ZeroValuesAtStartCommand() : Command ("zero-values-at-start")
  {
  }
  bool
  exec (Audio& audio)
  {
    printf ("zero-values-at-start: %d\n", audio.zero_values_at_start);
    return true;
  }
} zero_values_at_start_command;

class AttackCommand : public Command
{
public:
  AttackCommand() : Command ("attack")
  {
  }
  bool
  exec (Audio& audio)
  {
    printf ("start of attack: %.2f ms\n", audio.attack_start_ms);
    printf ("  end of attack: %.2f ms\n", audio.attack_end_ms);
    return true;
  }
} attack_command;

class SizeCommand : public Command
{
public:
  SizeCommand() : Command ("size")
  {
  }
  bool
  exec (Audio& audio)
  {
    size_t phase_bytes = 0, freq_bytes = 0, mag_bytes = 0, debug_samples_bytes = 0, original_fft_bytes = 0, noise_bytes = 0;
    for (size_t f = 0; f < audio.contents.size(); f++)
      {
        phase_bytes += audio.contents[f].phases.size() * sizeof (float);
        freq_bytes += audio.contents[f].freqs.size() * sizeof (float);
        mag_bytes += audio.contents[f].mags.size() * sizeof (float);
        debug_samples_bytes += audio.contents[f].debug_samples.size() * sizeof (float);
        original_fft_bytes += audio.contents[f].original_fft.size() * sizeof (float);
        noise_bytes += audio.contents[f].noise.size() * sizeof (float);
      }
    size_t original_samples_bytes = audio.original_samples.size() * sizeof (float);

    printf ("frequencies  : %zd bytes\n", freq_bytes);
    printf ("mags         : %zd bytes\n", mag_bytes);
    printf ("phases       : %zd bytes\n", phase_bytes);
    printf ("dbgsamples   : %zd bytes\n", debug_samples_bytes);
    printf ("orig_fft     : %zd bytes\n", original_fft_bytes);
    printf ("noise        : %zd bytes\n", noise_bytes);
    printf ("orig_samples : %zd bytes\n", original_samples_bytes);
    return true;
  }
} size_command;

class LoopParamsCommand : public Command
{
public:
  LoopParamsCommand() : Command ("loop-params")
  {
  }
  bool
  exec (Audio& audio)
  {
    printf ("frames: %zd\n", audio.contents.size());
    string loop_str;
    if (audio.loop_type_to_string (audio.loop_type, loop_str))
      printf ("loop type: %s\n", loop_str.c_str());
    else
      printf ("loop type: *unknown* (%d)\n", audio.loop_type);
    printf ("loop start: %d\n", audio.loop_start);
    printf ("loop end: %d\n", audio.loop_end);

    return true;
  }
} loop_params_command;

class NoiseParamsCommand : public Command
{
  int frame;
public:
  NoiseParamsCommand() : Command ("noise-params")
  {
  }
  bool
  parse_args (vector<string>& args)
  {
    if (args.size() == 1)
      {
        frame = atoi (args[0].c_str());
        return true;
      }
    return false;
  }
  bool
  exec (Audio& audio)
  {
    for (size_t i = 0; i < audio.contents[frame].noise.size(); i++)
      printf ("%f\n", audio.contents[frame].noise[i]);
    return true;
  }
  void
  usage (bool one_line)
  {
    printf ("<frame_no>\n");
  }
} noise_params_command;

class FrameCommand : public Command
{
  int frame;
public:
  FrameCommand() : Command ("frame")
  {
  }
  bool
  parse_args (vector<string>& args)
  {
    if (args.size() == 1)
      {
        frame = atoi (args[0].c_str());
        return true;
      }
    return false;
  }
  bool
  exec (Audio& audio)
  {
    int i = frame;
    size_t frame_size = audio.contents[i].debug_samples.size();
    vector<double> sines (frame_size);
    reconstruct (audio.contents[i], sines, audio.mix_freq);
    for (size_t n = 0; n < audio.contents[i].debug_samples.size(); n++)
      {
        double v = audio.contents[i].debug_samples[n];
        printf ("%zd %f %f %f\n", n, v, sines[n], v - sines[n]);
      }
    return true;
  }
  void
  usage (bool one_line)
  {
    printf ("<frame_no>\n");
  }
} frame_command;

class FrameParamsCommand : public Command
{
  int frame;
public:
  FrameParamsCommand() : Command ("frame-params")
  {
  }
  bool
  parse_args (vector<string>& args)
  {
    if (args.size() == 1)
      {
        frame = atoi (args[0].c_str());
        return true;
      }
    return false;
  }
  void
  usage (bool one_line)
  {
    printf ("<frame_no>\n");
  }
  bool
  exec (Audio& audio)
  {
    int i = frame;
    for(;;)
      {
        double maxm = 0;
        size_t maxp = 0;
        for (size_t partial = 0; partial < audio.contents[i].freqs.size(); partial++)
          {
            const double m = audio.contents[i].mags[partial];
            if (m > maxm)
              {
                maxm = m;
                maxp = partial;
              }
          }
        if (maxm > 0)
          {
            printf ("%f Hz: %f\n", audio.contents[i].freqs[maxp], bse_db_from_factor (maxm, -200));
            audio.contents[i].mags[maxp] = 0;
          }
        else
          {
            break;
          }
      }
    return true;
  }
} frame_params_command;

class TotalNoiseCommand : public Command
{
public:
  TotalNoiseCommand() : Command ("total-noise")
  {
  }
  bool
  exec (Audio& audio)
  {
    double total_noise = 0;
    double peak_noise  = 0;

    for (size_t f = 0; f < audio.contents.size(); f++)
      {
        for (size_t i = 0; i < audio.contents[f].noise.size(); i++)
          {
            double noise = audio.contents[f].noise[i];

            total_noise += noise;
            peak_noise   = max (peak_noise, noise);
          }
      }
    printf ("total-noise: %.17g\n", total_noise);
    printf ("peak-noise:  %.17g\n", peak_noise);
    return true;
  }
} total_noise_command;

class NanTestCommand : public Command
{
public:
  NanTestCommand() : Command ("nan-test")
  {
  }
  bool
  exec (Audio& audio)
  {
    int nan_phases = 0, nan_freqs = 0, nan_mags = 0, nan_ds = 0, nan_fft = 0, nan_noise = 0;

    for (size_t f = 0; f < audio.contents.size(); f++)
      {
        if (find_nan (audio.contents[f].phases))
          nan_phases++;
        if (find_nan (audio.contents[f].freqs))
          nan_freqs++;
        if (find_nan (audio.contents[f].mags))
          nan_mags++;
        if (find_nan (audio.contents[f].debug_samples))
          nan_ds++;
        if (find_nan (audio.contents[f].original_fft))
          nan_fft++;
        if (find_nan (audio.contents[f].noise))
          nan_noise++;
      }
    printf ("nan-phases:        %d\n", nan_phases);
    printf ("nan-freqs:         %d\n", nan_freqs);
    printf ("nan-mags:          %d\n", nan_mags);
    printf ("nan-debug-samples: %d\n", nan_ds);
    printf ("nan-original-fft:  %d\n", nan_fft);
    printf ("nan-noise:         %d\n", nan_noise);
    return true;
  }
} nan_test_command;

class OriginalSamplesCommand : public Command
{
public:
  OriginalSamplesCommand() : Command ("original-samples")
  {
  }
  bool
  exec (Audio& audio)
  {
    for (size_t i = 0; i < audio.original_samples.size(); i++)
      printf ("%.17g\n", audio.original_samples[i]);

    return true;
  }
} original_samples_command;

class FreqCommand : public Command
{
  double freq_min, freq_max;
public:
  FreqCommand() : Command ("freq")
  {
  }
  bool
  parse_args (vector<string>& args)
  {
    if (args.size() == 2)
      {
        freq_min = atof (args[0].c_str());
        freq_max = atof (args[1].c_str());
        return true;
      }
    return false;
  }
  bool
  exec (Audio& audio)
  {
    for (size_t i = 0; i < audio.contents.size(); i++)
      {
        const AudioBlock& block = audio.contents[i];
        for (size_t n = 0; n < block.freqs.size(); n++)
          {
            if (block.freqs[n] > freq_min && block.freqs[n] < freq_max)
              {
                printf ("%zd %f %f\n", i, block.freqs[n], block.mags[n]);
              }
          }
      }
    return true;
  }
  void
  usage (bool one_line)
  {
    printf ("<freq_min> <freq_max>\n");
  }
} freq_command;

class SpectrumCommand : public Command
{
  int frame;
public:
  SpectrumCommand() : Command ("spectrum")
  {
  }
  bool
  parse_args (vector<string>& args)
  {
    if (args.size() == 1)
      {
        frame = atoi (args[0].c_str());
        return true;
      }
    return false;
  }
  void
  usage (bool one_line)
  {
    printf ("<frame_no>\n");
  }
  bool
  exec (Audio& audio)
  {
    size_t frame_size = audio.frame_size_ms * audio.mix_freq / 1000;
    int i = frame;
    vector<double> spectrum;
    vector<double> sines (frame_size);

    reconstruct (audio.contents[i], sines, audio.mix_freq);

    /* compute block size from frame size (smallest 2^k value >= frame_size) */
    size_t block_size = 1;
    while (block_size < frame_size)
      block_size *= 2;

    // construct window
    vector<float> window (block_size);
    for (guint i = 0; i < window.size(); i++)
      {
        if (i < frame_size)
          window[i] = bse_window_cos (2.0 * i / frame_size - 1.0);
        else
          window[i] = 0;
      }

    // apply window to reconstructed signal
    sines.resize (block_size);
    for (guint i = 0; i < sines.size(); i++)
      sines[i] *= window[i];

    // zeropad
    const int    zeropad  = 4;
    sines.resize (block_size * zeropad);
    vector<double> out (block_size * zeropad);

    gsl_power2_fftar (block_size * zeropad, &sines[0], &out[0]);

    vector<double> sines_spectrum;
    for (size_t n = 0; n < audio.contents[i].original_fft.size(); n += 2)
      {
        double re = audio.contents[i].original_fft[n];
        double im = audio.contents[i].original_fft[n + 1];
        spectrum.push_back (sqrt (re * re + im * im));
        sines_spectrum.push_back (mag (out[n], out[n+1]));
      }
    for (size_t n = 0; n < spectrum.size(); n++)
      {
        double s = 0;
        for (size_t r = 0; r < 1; r++)
          {
            if (n + r < spectrum.size())
              s = std::max (s, spectrum[n + r]);
            if (r < n)
              s = std::max (s, spectrum[n - r]);
          }
        printf ("%f %f %f\n", n * 0.5 * audio.mix_freq / spectrum.size(), s, sines_spectrum[n]);
      }
    return true;
  }
} spectrum_command;

class AutoLoopCommand : public Command
{
  double percent;
public:
  AutoLoopCommand() : Command ("auto-loop")
  {
    set_need_save (true);
  }
  bool
  parse_args (vector<string>& args)
  {
    if (args.size() == 1)
      {
        percent = atof (args[0].c_str());
        if (percent < 0 || percent > 100)
          {
            fprintf (stderr, "bad loop percentage: %f\n", percent);
            return false;
          }
        return true;
      }
    return false;
  }
  void
  usage (bool one_line)
  {
    printf ("<percent>\n");
  }
  bool
  exec (Audio& audio)
  {
    int loop_point = audio.contents.size() * percent / 100;
    if (loop_point < 0)
      loop_point = 0;
    if (size_t (loop_point) >= (audio.contents.size() - 1))
      loop_point = audio.contents.size() - 1;
    audio.loop_type = Audio::LOOP_FRAME_FORWARD;
    audio.loop_start = loop_point;
    audio.loop_end = loop_point;
    return true;
  }
} auto_loop_command;

class TailLoopCommand : public Command
{
public:
  TailLoopCommand() : Command ("tail-loop")
  {
    set_need_save (true);
  }
  bool
  exec (Audio& audio)
  {
    int loop_point = -1;
    size_t frame_size = audio.frame_size_ms * audio.mix_freq / 1000;
    const int frame_step = audio.frame_step_ms * audio.mix_freq / 1000;

    // we need the largest frame that doesn't include any data beyond the original file end
    for (size_t i = 0; i < audio.contents.size(); i++)
      {
        if (i * frame_step + frame_size < size_t (audio.sample_count))
          loop_point = i;
      }
    audio.loop_type = Audio::LOOP_FRAME_FORWARD;
    audio.loop_start = loop_point;
    audio.loop_end = loop_point;

    return true;
  }
} tail_loop_command;

double
freq_ratio_to_cent (double freq_ratio)
{
  return log (freq_ratio) / log (2) * 1200;
}

class AutoTuneCommand : public Command
{
public:
  AutoTuneCommand() : Command ("auto-tune")
  {
  }
  bool
  exec (Audio& audio)
  {
    const double freq_min = audio.fundamental_freq * 0.8;
    const double freq_max = audio.fundamental_freq / 0.8;
    double freq_sum = 0, mag_sum = 0;

    for (size_t f = 0; f < audio.contents.size(); f++)
      {
        double position_percent = f * 100.0 / audio.contents.size();
        if (position_percent >= 40 && position_percent <= 60)
          {
            const AudioBlock& block = audio.contents[f];
            double best_freq = -1;
            double best_mag = 0;
            for (size_t n = 0; n < block.freqs.size(); n++)
              {
                if (block.freqs[n] > freq_min && block.freqs[n] < freq_max)
                  {
                    double m = block.mags[n];
                    if (m > best_mag)
                      {
                        best_mag = m;
                        best_freq = block.freqs[n];
                      }
                  }
              }
            if (best_mag > 0)
              {
                freq_sum += best_freq * best_mag;
                mag_sum += best_mag;
              }
          }
      }
    if (mag_sum > 0)
      {
        double fundamental_freq = freq_sum / mag_sum;
        printf ("%.17g %.17g  %.3f cent\n", audio.fundamental_freq, fundamental_freq,
                                            freq_ratio_to_cent (audio.fundamental_freq / fundamental_freq));
        audio.fundamental_freq = fundamental_freq;

        set_need_save (true);
      }
    return true;
  }
} auto_tune_command;

class TuneAllFramesCommand : public Command
{
public:
  TuneAllFramesCommand() : Command ("tune-all-frames")
  {
  }
  bool
  exec (Audio& audio)
  {
    const double freq_min = audio.fundamental_freq * 0.8;
    const double freq_max = audio.fundamental_freq / 0.8;

    for (size_t f = 0; f < audio.contents.size(); f++)
      {
        AudioBlock& block = audio.contents[f];
        double max_mag = -1;
        size_t max_p = 0;
        for (size_t p = 0; p < block.mags.size(); p++)
          {
            if (block.freqs[p] > freq_min && block.freqs[p] < freq_max && block.mags[p] > max_mag)
              {
                max_mag = block.mags[p];
                max_p = p;
              }
          }
        if (max_mag > 0)
          {
            double tune_factor = audio.fundamental_freq / block.freqs[max_p];
            for (size_t p = 0; p < block.freqs.size(); p++)
              {
                block.freqs[p] *= tune_factor;
                set_need_save (true);
              }
          }
      }
    return true;
  }
} tune_all_frames_command;

static void
normalize_energy (double energy, Audio& audio)
{
  const double target_energy = 0.05;
  const double norm = sqrt (target_energy / energy);
  printf ("avg_energy: %.17g\n", energy);
  printf ("norm:       %.17g\n", norm);
  for (size_t f = 0; f < audio.contents.size(); f++)
    {
      vector<float>& mags = audio.contents[f].mags;
      for (size_t i = 0; i < mags.size(); i++)
        mags[i] *= norm;

      vector<float>& noise = audio.contents[f].noise;
      for (size_t i = 0; i < noise.size(); i++)
        noise[i] *= norm * norm;
    }
}

class AutoVolumeCommand : public Command
{
  double percent;
public:
  AutoVolumeCommand() : Command ("auto-volume")
  {
    set_need_save (true);
  }
  bool
  parse_args (vector<string>& args)
  {
    if (args.size() == 1)
      {
        percent = atof (args[0].c_str());
        if (percent < 0 || percent > 100)
          {
            fprintf (stderr, "bad volume percentage: %f\n", percent);
            return false;
          }
        return true;
      }
    return false;
  }
  void
  usage (bool one_line)
  {
    printf ("<percent>\n");
  }
  bool
  exec (Audio& audio)
  {
    double energy = compute_energy (audio, percent, false);
    normalize_energy (energy, audio);
    return true;
  }
} auto_volume_command;

class AutoVolumeFromLoopCommand : public Command
{
public:
  AutoVolumeFromLoopCommand() : Command ("auto-volume-from-loop")
  {
    set_need_save (true);
  }
  bool
  exec (Audio& audio)
  {
    double energy = compute_energy (audio, /* dummy */ 50, true);
    normalize_energy (energy, audio);
    return true;
  }
} auto_volume_from_loop_command;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  if (argc < 3)
    {
      printf ("usage: smtool <sm_file> <mode> [ <mode_specific_args> ]\n");
      printf ("\n");
      printf ("mode specific args:\n\n");

      for (vector<Command *>::iterator ci = Command::registry()->begin(); ci != Command::registry()->end(); ci++)
        {
          printf ("  smtool <sm_file> %s ", (*ci)->mode().c_str());
          (*ci)->usage (true);
        }
      return 1;
    }

  const string& mode = argv[2];

  /* figure out file type (we support SpectMorph::WavSet and SpectMorph::Audio) */
  InFile *file = new InFile (argv[1]);
  if (!file->open_ok())
    {
      fprintf (stderr, "%s: can't open input file: %s\n", argv[0], argv[1]);
      exit (1);
    }
  string file_type = file->file_type();
  delete file;

  Audio *audio = NULL;
  WavSet *wav_set = NULL;
  if (file_type == "SpectMorph::Audio")
    {
      audio = new Audio;
      load_or_die (*audio, argv[1], mode);
    }
  else if (file_type == "SpectMorph::WavSet")
    {
      wav_set = new WavSet;
      BseErrorType error = wav_set->load (argv[1]);
      if (error)
        {
          fprintf (stderr, "smtool: can't load file: %s\n", argv[1]);
          return 1;
        }
    }
  else
    {
      g_printerr ("unknown file_type: %s\n", file_type.c_str());
      return 1;
    }

  bool need_save = false;
  bool found_command = false;

  vector<string> args;
  for (int i = 3; i < argc; i++)
    args.push_back (argv[i]);

  for (vector<Command *>::iterator ci = Command::registry()->begin(); ci != Command::registry()->end(); ci++)
    {
      Command *cmd = *ci;
      if (cmd->mode() == mode)
        {
          assert (!found_command);
          found_command = true;

          if (!cmd->parse_args (args))
            {
              printf ("usage: smtool <sm_file> %s ", cmd->mode().c_str());
              cmd->usage (true);
              return 1;
            }
          if (audio)
            cmd->exec (*audio);
          if (wav_set)
            {
              set<Audio *> done;

              for (vector<WavSetWave>::iterator wi = wav_set->waves.begin(); wi != wav_set->waves.end(); wi++)
                {
                  printf ("## midi_note=%d channel=%d velocity_range=%d..%d\n", wi->midi_note, wi->channel,
                          wi->velocity_range_min, wi->velocity_range_max);
                  if (done.find (wi->audio) != done.end())
                    {
                      printf ("## ==> skipped (was processed earlier)\n");
                    }
                  else
                    {
                      cmd->exec (*wi->audio);
                      done.insert (wi->audio);
                    }
                }
            }

          need_save = cmd->need_save();
        }
    }
  if (!found_command)
    {
      g_printerr ("unknown mode: %s\n", mode.c_str());
    }
  if (need_save)
    {
      if (audio && audio->save (argv[1]) != BSE_ERROR_NONE)
        {
          fprintf (stderr, "error saving audio file: %s\n", argv[1]);
          return 1;
        }
      if (wav_set && wav_set->save (argv[1]) != BSE_ERROR_NONE)
        {
          fprintf (stderr, "error saving wavset file: %s\n", argv[1]);
          return 1;
        }
    }
  if (wav_set)
    {
      delete wav_set;
      wav_set = 0;
    }
  if (audio)
    {
      delete audio;
      audio = 0;
    }
}
