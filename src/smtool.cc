// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <vector>
#include <stdio.h>
#include <assert.h>
#include <QtGlobal>

#include "smaudio.hh"
#include "smwavset.hh"
#include "smlivedecoder.hh"
#include "smmain.hh"
#include "sminfile.hh"
#include "smutils.hh"
#include "smlpc.hh"
#include "smfft.hh"

using namespace SpectMorph;
using std::vector;
using std::min;
using std::max;
using std::string;
using std::set;
using std::complex;

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
             const Audio&    audio)
{
  for (size_t partial = 0; partial < audio_block.freqs.size(); partial++)
    {
      double f     = audio_block.freqs_f (partial) * audio.fundamental_freq;
      double mag   = audio_block.mags_f (partial);
      double phase = audio_block.phases_f (partial);

      // do a phase optimal reconstruction of that partial
      for (size_t n = 0; n < signal.size(); n++)
        {
          signal[n] += sin (phase) * mag;
          phase += f / audio.mix_freq * 2.0 * M_PI;
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

  Error error = audio.load (filename, load_options);
  if (error != 0)
    {
      fprintf (stderr, "can't load file: %s\n",filename.c_str());
      exit (1);
    }
}

static bool
find_nan (vector<float>& data)
{
  for (size_t x = 0; x < data.size(); x++)
    if (std::isnan (data[x]))
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
  decoder.process (samples.size(), nullptr, &samples[0]);

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
  string            m_mode;
  bool              m_need_save;
  const WavSetWave *m_wave;
public:
  static vector<Command *> *registry();
  Command (const string& mode)
  {
    registry()->push_back (this);
    m_mode = mode;
    m_need_save = false;
    m_wave = NULL;
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
  string mode() const
  {
    return m_mode;
  }
  bool
  need_save() const
  {
    return m_need_save;
  }
  void
  set_need_save (bool s)
  {
    m_need_save = s;
  }
  const WavSetWave *
  wave() const
  {
    return m_wave;
  }
  void
  set_wave (const WavSetWave *wave)
  {
    m_wave = wave;
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
    sm_printf ("avg_energy: %.17g\n", energy);
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
    sm_printf ("fundamental-freq: %f\n", audio.fundamental_freq);
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
    sm_printf ("mix-freq: %f\n", audio.mix_freq);
    return true;
  }
} mix_freq_command;

class StatsCommand : public Command
{
public:
  StatsCommand() : Command ("stats")
  {
  }
  bool
  exec (Audio& audio)
  {
    const WavSetWave *w = wave();

    double mag_weight = 0, mag_partials = 0;
    for (size_t f = 0; f < audio.contents.size(); f++)
      {
        const AudioBlock& block = audio.contents[f];

        double mag = 0;
        for (size_t i = 0; i < block.freqs.size(); i++)
          mag += block.mags_f (i);

        /* give higher weight to louder audio blocks */
        mag_weight   += mag;
        mag_partials += mag * block.freqs.size();
      }
    mag_partials /= mag_weight;
    sm_printf ("%d %d %f\n", w ? w->midi_note : -1, int (audio.mix_freq + 0.5), mag_partials);
    return true;
  }
} stats_command;

class SampleCountCommand : public Command
{
public:
  SampleCountCommand() : Command ("sample-count")
  {
  }
  bool
  exec (Audio& audio)
  {
    sm_printf ("sample-count: %d\n", audio.sample_count);
    return true;
  }
} sample_count_command;

class ZeroValuesAtStartCommand : public Command
{
public:
  ZeroValuesAtStartCommand() : Command ("zero-values-at-start")
  {
  }
  bool
  exec (Audio& audio)
  {
    sm_printf ("zero-values-at-start: %d\n", audio.zero_values_at_start);
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
    sm_printf ("start of attack: %.2f ms\n", audio.attack_start_ms);
    sm_printf ("  end of attack: %.2f ms\n", audio.attack_end_ms);
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
        phase_bytes += audio.contents[f].phases.size() * sizeof (uint16_t);
        freq_bytes += audio.contents[f].freqs.size() * sizeof (uint16_t);
        mag_bytes += audio.contents[f].mags.size() * sizeof (uint16_t);
        debug_samples_bytes += audio.contents[f].debug_samples.size() * sizeof (float);
        original_fft_bytes += audio.contents[f].original_fft.size() * sizeof (float);
        noise_bytes += audio.contents[f].noise.size() * sizeof (uint16_t);
      }
    size_t original_samples_bytes = audio.original_samples.size() * sizeof (float);

    sm_printf ("frequencies  : %zd bytes\n", freq_bytes);
    sm_printf ("mags         : %zd bytes\n", mag_bytes);
    sm_printf ("phases       : %zd bytes\n", phase_bytes);
    sm_printf ("dbgsamples   : %zd bytes\n", debug_samples_bytes);
    sm_printf ("orig_fft     : %zd bytes\n", original_fft_bytes);
    sm_printf ("noise        : %zd bytes\n", noise_bytes);
    sm_printf ("orig_samples : %zd bytes\n", original_samples_bytes);

    size_t total_bytes = (freq_bytes + mag_bytes + phase_bytes + noise_bytes);
    sm_printf ("data rate    : %.2f K/s\n", total_bytes / 1024.0 / (audio.sample_count / audio.mix_freq));
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
    sm_printf ("frames: %zd\n", audio.contents.size());
    string loop_str;
    if (audio.loop_type_to_string (audio.loop_type, loop_str))
      sm_printf ("loop type: %s\n", loop_str.c_str());
    else
      sm_printf ("loop type: *unknown* (%d)\n", audio.loop_type);
    sm_printf ("loop start: %d\n", audio.loop_start);
    sm_printf ("loop end: %d\n", audio.loop_end);

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
      sm_printf ("%.7g\n", audio.contents[frame].noise_f (i));
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
    reconstruct (audio.contents[i], sines, audio);
    for (size_t n = 0; n < audio.contents[i].debug_samples.size(); n++)
      {
        double v = audio.contents[i].debug_samples[n];
        sm_printf ("%zd %f %f %f\n", n, v, sines[n], v - sines[n]);
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
        int    maxm = 0;
        size_t maxp = 0;
        for (size_t partial = 0; partial < audio.contents[i].freqs.size(); partial++)
          {
            const int m = audio.contents[i].mags[partial];
            if (m > maxm)
              {
                maxm = m;
                maxp = partial;
              }
          }
        if (maxm > 0)
          {
            const double freq = audio.contents[i].freqs_f (maxp) * audio.fundamental_freq;
            const double mag_factor = audio.contents[i].mags_f (maxp);
            const double mag_db = db_from_factor (mag_factor, -200);

            sm_printf ("%f Hz: %f\n", freq, mag_db);
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
            const double noise = audio.contents[f].noise_f (i);

            total_noise += noise;
            peak_noise   = max (peak_noise, noise);
          }
      }
    sm_printf ("total-noise: %.17g\n", total_noise);
    sm_printf ("peak-noise:  %.17g\n", peak_noise);
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
    int nan_ds = 0, nan_fft = 0;

    for (size_t f = 0; f < audio.contents.size(); f++)
      {
        if (find_nan (audio.contents[f].debug_samples))
          nan_ds++;
        if (find_nan (audio.contents[f].original_fft))
          nan_fft++;
      }
    sm_printf ("nan-debug-samples: %d\n", nan_ds);
    sm_printf ("nan-original-fft:  %d\n", nan_fft);
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
      sm_printf ("%.17g\n", audio.original_samples[i]);

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
            const double freq = block.freqs_f (n) * audio.fundamental_freq;

            if (freq > freq_min && freq < freq_max)
              {
                sm_printf ("%zd %f %f\n", i, freq, block.mags_f (n));
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

    reconstruct (audio.contents[i], sines, audio);

    /* compute block size from frame size (smallest 2^k value >= frame_size) */
    size_t block_size = 1;
    while (block_size < frame_size)
      block_size *= 2;

    // construct window
    vector<float> window (block_size);
    for (guint i = 0; i < window.size(); i++)
      {
        if (i < frame_size)
          window[i] = window_cos (2.0 * i / frame_size - 1.0);
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

    float *fft_in = FFT::new_array_float (sines.size());
    float *fft_out = FFT::new_array_float (sines.size());

    std::copy (sines.begin(), sines.end(), fft_in);
    FFT::fftar_float (sines.size(), fft_in, fft_out);
    std::copy (fft_out, fft_out + sines.size(), out.begin());

    FFT::free_array_float (fft_out);
    FFT::free_array_float (fft_in);

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
        sm_printf ("%f %f %f\n", n * 0.5 * audio.mix_freq / spectrum.size(), s, sines_spectrum[n]);
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
    const double freq_min = 0.8;
    const double freq_max = 1.25;
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
                const double freq = block.freqs_f (n);

                if (freq > freq_min && freq < freq_max)
                  {
                    const double m = block.mags_f (n);
                    if (m > best_mag)
                      {
                        best_mag = m;
                        best_freq = block.freqs_f (n);
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
        double tune_factor = 1.0 / (freq_sum / mag_sum);
        double input_fundamental_freq = audio.fundamental_freq / tune_factor;
        for (size_t f = 0; f < audio.contents.size(); f++)
          {
            AudioBlock& block = audio.contents[f];

            for (size_t n = 0; n < block.freqs.size(); n++)
              {
                const double freq = block.freqs_f (n) * tune_factor;
                block.freqs[n] = sm_freq2ifreq (freq);
              }
          }
        sm_printf ("%.17g  %.17g  %.3f cent\n", audio.fundamental_freq, input_fundamental_freq, freq_ratio_to_cent (tune_factor));

        set_need_save (true);
      }
    return true;
  }
} auto_tune_command;

class TuneAllFramesCommand : public Command
{
  int fundamental_est_n; /* number of partials to use for fundamental estimation in range [1,3] */
public:
  TuneAllFramesCommand() : Command ("tune-all-frames")
  {
  }
  bool
  parse_args (vector<string>& args)
  {
    if (args.size() == 1)
      {
        fundamental_est_n = atoi (args[0].c_str());
        assert (fundamental_est_n >= 1 && fundamental_est_n <= 3);
        return true;
      }
    return false;
  }
  void
  usage (bool one_line)
  {
    printf ("<n_partials>\n");
  }
  void
  update_fundamental_estimate (int n, const AudioBlock& block, double freq_min, double freq_max, double& f_out, double& m_out)
  {
    if (n > fundamental_est_n) /* use this partial for fundamental estimation? */
      return;

    double freq = 0, mag = 0;

    for (size_t p = 0; p < block.mags.size(); p++)
      {
        if (block.freqs_f (p) > freq_min && block.freqs_f (p) < freq_max && block.mags_f (p) > mag)
          {
            mag = block.mags_f (p);
            freq = block.freqs_f (p) / n;
          }
      }
    if (mag > 0)
      {
        m_out += mag;
        f_out += freq * mag;
      }
  }
  bool
  exec (Audio& audio)
  {
    double weighted_tune_factor = 0, mag_weight = 0; /* gather statistics to output average tuning */
    for (size_t f = 0; f < audio.contents.size(); f++)
      {
        AudioBlock& block = audio.contents[f];

        double est_freq = 0, est_mag = 0;

        update_fundamental_estimate (1, block, 0.8, 1.25, est_freq, est_mag);
        update_fundamental_estimate (2, block, 1.5, 2.5,  est_freq, est_mag);
        update_fundamental_estimate (3, block, 2.5, 3.5,  est_freq, est_mag);

        if (est_mag > 0)
          {
            est_freq /= est_mag;
            const double tune_factor = 1 / est_freq;

            /* debug printf ("TAF %f %f\n", audio.fundamental_freq, freq_ratio_to_cent (tune_factor)); */

            for (size_t p = 0; p < block.freqs.size(); p++)
              {
                const double freq = block.freqs_f (p) * tune_factor;
                block.freqs[p] = sm_freq2ifreq (freq);
                set_need_save (true);

                /* assume orthogonal waves -> mags can be added */
                weighted_tune_factor += block.mags_f (p) * tune_factor;
                mag_weight += block.mags_f (p);
              }
          }
      }

    /* we can't give the exact tuning for all frames, so we compute a crude approximation
     * for the input frequency and "average" tuning factor, using a higher weight
     * for louder frames
     */
    weighted_tune_factor /= mag_weight;
    const double input_fundamental_freq = audio.fundamental_freq / weighted_tune_factor;
    sm_printf ("%.17g  %.17g  %.3f cent\n", audio.fundamental_freq, input_fundamental_freq, freq_ratio_to_cent (weighted_tune_factor));
    return true;
  }
} tune_all_frames_command;

static void
normalize_factor (double norm, Audio& audio)
{
  const int    norm_delta_idb   = sm_factor2delta_idb (norm);

  for (size_t f = 0; f < audio.contents.size(); f++)
    {
      vector<uint16_t>& mags = audio.contents[f].mags;
      for (size_t i = 0; i < mags.size(); i++)
        mags[i] = qBound<int> (0, mags[i] + norm_delta_idb, 65535);

      vector<uint16_t>& noise = audio.contents[f].noise;
      for (size_t i = 0; i < noise.size(); i++)
        noise[i] = qBound<int> (0, noise[i] + norm_delta_idb, 65535);
    }

  // store normalization in order to replay original samples normalized
  const double samples_factor = db_to_factor (audio.original_samples_norm_db);
  audio.original_samples_norm_db = db_from_factor (samples_factor * norm, -200);
}

static void
normalize_energy (double energy, Audio& audio)
{
  const double target_energy = 0.05;
  const double norm = sqrt (target_energy / energy);
  sm_printf ("avg_energy: %.17g\n", energy);
  sm_printf ("norm:       %.17g\n", norm);

  normalize_factor (norm, audio);
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

class GlobalVolumeCommand : public Command
{
  double norm_db;
public:
  GlobalVolumeCommand() : Command ("global-volume")
  {
    set_need_save (true);
  }
  bool
  parse_args (vector<string>& args)
  {
    if (args.size() == 1)
      {
        norm_db = atof (args[0].c_str());
        return true;
      }
    return false;
  }
  void
  usage (bool one_line)
  {
    printf ("<db>\n");
  }
  bool
  exec (Audio& audio)
  {
    normalize_factor (db_to_factor (norm_db), audio);
    return true;
  }
} global_volume_command;


class StripCommand : public Command
{
public:
  StripCommand() : Command ("strip")
  {
    set_need_save (true);
  }
  bool
  exec (Audio& audio)
  {
    /* it would be nice if we could have options like --keep-samples || --strip-lpc
     * but for now we just do the default thing
     */
    for (size_t i = 0; i < audio.contents.size(); i++)
      {
        audio.contents[i].debug_samples.clear();
        audio.contents[i].original_fft.clear();
      }
    audio.original_samples.clear();
    return true;
  }
} strip_command;

class LPCCommand : public Command
{
  int lpc_order;
public:
  LPCCommand() : Command ("lpc")
  {
    set_need_save (true);
  }
  bool
  parse_args (vector<string>& args)
  {
    if (args.size() == 1)
      {
        lpc_order = atoi (args[0].c_str());

        assert ((lpc_order & 1) == 0);  // lpc order must be even
        assert (lpc_order >= 2);        // ... and at least 2

        return true;
      }
    return false;
  }
  void
  usage (bool one_line)
  {
    printf ("<lpc_order>\n");
  }
  bool
  exec (Audio& audio)
  {
    const size_t frame_size = audio.frame_size_ms * LPC::MIX_FREQ / 1000;

    for (AudioBlock& audio_block : audio.contents)
      {
        AlignedArray<float,16> signal (frame_size);
        for (size_t i = 0; i < audio_block.freqs.size(); i++)
          {
            const double freq = audio_block.freqs_f (i) * audio.fundamental_freq;
            const double mag = audio_block.mags_f (i);

            if (freq < LPC::MIX_FREQ / 2) // ignore freqs > nyquist
              {
                VectorSinParams params;
                params.mix_freq = LPC::MIX_FREQ;
                params.freq = freq;
                params.phase = 0;
                params.mag = mag;
                params.mode = VectorSinParams::ADD;

                fast_vector_sinf (params, &signal[0], &signal[frame_size]);
              }
          }
        vector<double> lpc (lpc_order);

        LPC::compute_lpc (lpc, &signal[0], &signal[frame_size]);

        // make LPC filter stable
        vector< complex<double> > roots;
        bool good_roots = LPC::find_roots (lpc, roots);
        assert (good_roots);
        LPC::make_stable_roots (roots);
        LPC::roots2lpc (roots, lpc);

        LPC::lpc2lsf (lpc, audio_block.lpc_lsf_p, audio_block.lpc_lsf_q);
      }
    return true;
  }
} lpc_command;

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
      Error error = wav_set->load (argv[1]);
      if (error != 0)
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
                  sm_printf ("## midi_note=%d channel=%d velocity_range=%d..%d\n", wi->midi_note, wi->channel,
                             wi->velocity_range_min, wi->velocity_range_max);
                  if (done.find (wi->audio) != done.end())
                    {
                      sm_printf ("## ==> skipped (was processed earlier)\n");
                    }
                  else
                    {
                      cmd->set_wave (&*wi);
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
      return 1;
    }
  if (need_save)
    {
      if (audio && audio->save (argv[1]) != Error::NONE)
        {
          fprintf (stderr, "error saving audio file: %s\n", argv[1]);
          return 1;
        }
      if (wav_set && wav_set->save (argv[1]) != Error::NONE)
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
