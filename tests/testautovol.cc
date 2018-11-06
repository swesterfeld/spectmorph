// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smlivedecoder.hh"
#include "smutils.hh"
#include "smmain.hh"
#include "smaudiotool.hh"

using namespace SpectMorph;

using std::vector;

class ConstBlockSource : public LiveDecoderSource
{
  Audio      my_audio;
  AudioBlock my_audio_block;
public:
  ConstBlockSource (const AudioBlock& block)
    : my_audio_block (block)
  {
    my_audio.frame_size_ms = 40;
    my_audio.frame_step_ms = 10;
    my_audio.attack_start_ms = 10;
    my_audio.attack_end_ms = 20;
    my_audio.zeropad = 4;
    my_audio.loop_type = Audio::LOOP_NONE;

    if (my_audio_block.noise.empty())
      {
        my_audio_block.noise.resize (32); // all 0, no noise
      }
  }
  void retrigger (int channel, float freq, int midi_velocity, float mix_freq)
  {
    my_audio.mix_freq = mix_freq;
    my_audio.fundamental_freq = freq;
  }
  Audio *audio()
  {
    return &my_audio;
  }
  AudioBlock *audio_block (size_t index)
  {
    return &my_audio_block;
  }
};

void
push_partial_f (AudioBlock& block, double freq_f, double mag_f, double phase_f)
{
  block.freqs.push_back (sm_freq2ifreq (freq_f));
  block.mags.push_back (sm_factor2idb (mag_f));
  block.phases.push_back (sm_bound<int> (0, sm_round_positive (phase_f / 2 / M_PI * 65536), 65535));
}

void
set_noise_f (AudioBlock& block, int band, double val)
{
  block.noise[band] = sm_factor2idb (val);
}

double
energy (const vector<float>& samples)
{
  double e = 0;
  for (float s : samples)
    e += s * s;

  return e / samples.size();
}

void
run_test (int n_sines, int n_noise, int mix_freq)
{
  AudioBlock audio_block;
  if (n_sines > 0)
    push_partial_f (audio_block, 1, 0.42, 0);

  if (n_sines > 1)
    push_partial_f (audio_block, 1.7, 0.1, 0);

  if (n_sines > 2)
    push_partial_f (audio_block, 3.7, 0.25, 0);

  audio_block.noise.resize (32);

  if (n_noise > 0)
    set_noise_f (audio_block, 4, 0.1);

  if (n_noise > 1)
    set_noise_f (audio_block, 12, 0.1);

  if (n_noise > 2)
    set_noise_f (audio_block, 26, 0.1);

  ConstBlockSource source (audio_block);
  LiveDecoder live_decoder (&source);
  //live_decoder.set_noise_seed (42);
  live_decoder.retrigger (0, 440, 127, mix_freq);

  vector<float> samples (mix_freq * 10);
  live_decoder.process (samples.size(), nullptr, &samples[0]);
  samples.erase (samples.begin(), samples.begin() + mix_freq);

  AudioTool::Block2Energy b2e (mix_freq);

  const double e = energy (samples);
  const double block_e = b2e.energy (audio_block);

  sm_printf ("energy = %f | block_energy = %f | factor = %f\n", e, block_e, e / block_e);
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  for (int n_sines = 0; n_sines <= 3; n_sines++)
    {
      for (int n_noise = 0; n_noise <= 3; n_noise++)
        {
          for (int high_mix = 0; high_mix <= 1; high_mix++)
            {
              if (n_sines || n_noise)
                run_test (n_sines, n_noise, high_mix ? 96000 : 48000);
            }
        }
    }
}
