// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smaudiotool.hh"
#include "smnoisebandpartition.hh"

using namespace SpectMorph;

using std::vector;

AudioTool::Block2Energy::Block2Energy (double mix_freq)
{
  NoiseBandPartition partition (32, 4096, mix_freq);

  for (size_t i = 0; i < 32; i++)
    noise_factors.push_back (mix_freq * partition.bins_per_band (i) / 4096.);
}

double
AudioTool::Block2Energy::energy (const AudioBlock& block)
{
  g_return_val_if_fail (block.noise.size() == noise_factors.size(), 0);

  double e = 0;
  for (size_t i = 0; i < block.mags.size(); i++)
    e += 0.5 * block.mags_f (i) * block.mags_f (i);

  for (size_t i = 0; i < block.noise.size(); i++)
    e += block.noise_f (i) * block.noise_f (i) * noise_factors[i];

  return e;
}

double
AudioTool::compute_energy (const Audio& audio)
{
  double e = 0;
  double e_norm = 0;

  size_t start = 0;
  size_t end   = audio.contents.size();

  if (audio.loop_type == Audio::LoopType::LOOP_FRAME_FORWARD ||
      audio.loop_type == Audio::LoopType::LOOP_FRAME_PING_PONG)
    {
      start = sm_bound<int> (start, audio.loop_start, end);

      // last loop block is also used for normalization => (loop_end + 1)
      end   = sm_bound<int> (start, audio.loop_end + 1, end);
    }

  AudioTool::Block2Energy block2energy (48000);

  for (size_t b = start; b < end; b++)
    {
      e += block2energy.energy (audio.contents[b]);
      e_norm += 1;
    }
  return e / e_norm;
}

void
AudioTool::normalize_factor (double norm, Audio& audio)
{
  const int    norm_delta_idb   = sm_factor2delta_idb (norm);

  for (size_t f = 0; f < audio.contents.size(); f++)
    {
      vector<uint16_t>& mags = audio.contents[f].mags;
      for (size_t i = 0; i < mags.size(); i++)
        mags[i] = sm_bound<int> (0, mags[i] + norm_delta_idb, 65535);

      vector<uint16_t>& noise = audio.contents[f].noise;
      for (size_t i = 0; i < noise.size(); i++)
        noise[i] = sm_bound<int> (0, noise[i] + norm_delta_idb, 65535);
    }

  // store normalization in order to replay original samples normalized
  const double samples_factor = db_to_factor (audio.original_samples_norm_db);
  audio.original_samples_norm_db = db_from_factor (samples_factor * norm, -200);
}

void
AudioTool::normalize_energy (double energy, Audio& audio)
{
  const double target_energy = 0.05;
  const double norm = sqrt (target_energy / energy);

  normalize_factor (norm, audio);
}

bool
AudioTool::get_auto_tune_factor (Audio& audio, double& tune_factor)
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
      tune_factor = 1.0 / (freq_sum / mag_sum);
      return true;
    }
  else
    {
      tune_factor = 1.0;
      return false;
    }
}

void
AudioTool::apply_auto_tune_factor (Audio& audio, double tune_factor)
{
  for (size_t f = 0; f < audio.contents.size(); f++)
    {
      AudioBlock& block = audio.contents[f];

      for (size_t n = 0; n < block.freqs.size(); n++)
        {
          const double freq = block.freqs_f (n) * tune_factor;
          block.freqs[n] = sm_freq2ifreq (freq);
        }
    }
}
