// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
      start = std::clamp<int> (audio.loop_start, start, end);

      // last loop block is also used for normalization => (loop_end + 1)
      end   = std::clamp<int> (audio.loop_end + 1, start, end);
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
  const int norm_delta_idb = sm_factor2delta_idb (norm);

  for (size_t f = 0; f < audio.contents.size(); f++)
    {
      vector<uint16_t>& mags = audio.contents[f].mags;
      for (size_t i = 0; i < mags.size(); i++)
        mags[i] = std::clamp (mags[i] + norm_delta_idb, 0, 65535);

      vector<uint16_t>& noise = audio.contents[f].noise;
      for (size_t i = 0; i < noise.size(); i++)
        noise[i] = std::clamp (noise[i] + norm_delta_idb, 0, 65535);

      vector<uint16_t>& env = audio.contents[f].env;
      for (size_t i = 0; i < env.size(); i++)
        env[i] = std::clamp (env[i] + norm_delta_idb, 0, 65535);
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
AudioTool::apply_auto_tune_factor (AudioBlock& audio_block, double tune_factor)
{
  for (size_t n = 0; n < audio_block.freqs.size(); n++)
    {
      const double freq = audio_block.freqs_f (n) * tune_factor;
      audio_block.freqs[n] = sm_freq2ifreq (freq);
    }
  audio_block.env_f0 *= tune_factor;
}

void
AudioTool::apply_auto_tune_factor (Audio& audio, double tune_factor)
{
  for (auto& audio_block : audio.contents)
    apply_auto_tune_factor (audio_block, tune_factor);
}

void
AudioTool::auto_tune_smooth (Audio& audio, int partials, double smooth_ms, double smooth_percent)
{
  vector<double> freq_vector;

  for (const auto& block : audio.contents)
    freq_vector.push_back (block.estimate_fundamental (partials));

  for (size_t f = 0; f < audio.contents.size(); f++)
    {
      double avg = 0;
      int count = 0;
      for (size_t j = 0; j < audio.contents.size(); j++)
        {
          double distance_ms = audio.frame_step_ms * fabs (double (f) - double (j));
          if (distance_ms < smooth_ms)
            {
              avg += freq_vector[j];
              count += 1;
            }
        }

      double smooth_freq = avg / count;
      double interp = smooth_percent / 100;
      double dest_freq = (freq_vector[f] / smooth_freq - 1) * interp + 1;
      const double tune_factor = dest_freq / freq_vector[f];

      apply_auto_tune_factor (audio.contents[f], tune_factor);
    }
}

void
AudioTool::FundamentalEst::add_partial (double freq, double mag)
{
  auto update_estimate = [&] (int n, double freq_min, double freq_max)
    {
      if (freq > freq_min && freq < freq_max && mag > m_best_mag[n])
        {
          m_best_freq[n] = freq / n;
          m_best_mag[n] = mag;
        }
    };
  update_estimate (1, 0.8, 1.25);
  update_estimate (2, 1.5, 2.5);
  update_estimate (3, 2.5, 3.5);
}

double
AudioTool::FundamentalEst::fundamental (int n_partials) const
{
  g_return_val_if_fail (n_partials >= 1 && n_partials <= 3, 1.0);

  double fsum = 0, msum = 0;
  for (int i = 1; i <= n_partials; i++)
    {
      fsum += m_best_freq[i] * m_best_mag[i];
      msum += m_best_mag[i];
    }

  if (msum > 0)
    return fsum / msum;
  else
    return 1;
}
