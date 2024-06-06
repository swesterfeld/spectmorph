// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smformantcorrection.hh"

using namespace SpectMorph;

using std::vector;
using std::max;
using std::min;

FormantCorrection::FormantCorrection()
{
  detune_factors.reserve (RESYNTH_MAX_PARTIALS);
  next_detune_factors.reserve (RESYNTH_MAX_PARTIALS);
}

void
FormantCorrection::set_ratio (float new_ratio)
{
  ratio = new_ratio;
}

void
FormantCorrection::set_mode (Mode new_mode)
{
  mode = new_mode;
}

void
FormantCorrection::set_fuzzy_resynth (float new_fuzzy_resynth)
{
  /* non-linear mapping from percent to cent: allow better control for small cent values */
  double f = new_fuzzy_resynth * 0.01;
  fuzzy_resynth = (f + 2 * f * f) * 16 / 3;
}

void
FormantCorrection::set_max_partials (int new_max_partials)
{
  max_partials = new_max_partials;
}

void
FormantCorrection::retrigger()
{
  detune_factors.clear();
  next_detune_factors.clear();
  fuzzy_frac = detune_random.random_double_range (0, 1);
  fuzzy_resynth_freq = detune_random.random_double_range (RESYNTH_MIN_FREQ, RESYNTH_MAX_FREQ);
}

void
FormantCorrection::gen_detune_factors (vector<float>& factors, size_t partials)
{
  if (factors.size() >= partials)
    return;

  size_t start = max<size_t> (1, factors.size());
  factors.resize (partials);

  const double max_fuzzy_resynth_delta = 0.029302236643492028782; // 50 cent (2**(50./1200) - 1)
  const double fuzzy_high = exp2 (fuzzy_resynth / 1200.0);
  const double fuzzy_low = 1 / fuzzy_high;

  for (size_t i = start; i < partials; i++)
    {
      double fuzzy_high_bound = 1 + max_fuzzy_resynth_delta / i;
      double fuzzy_low_bound = 1 / fuzzy_high_bound;

      factors[i] = detune_random.random_double_range (max (fuzzy_low, fuzzy_low_bound), min (fuzzy_high, fuzzy_high_bound));
    }
}

void
FormantCorrection::advance (double time_ms)
{
  fuzzy_frac += 0.001 * time_ms * fuzzy_resynth_freq;
}

void
FormantCorrection::process_block (const AudioBlock& in_block, RTAudioBlock& out_block)
{
  if (mode == MODE_REPITCH)
    {
      out_block.assign (in_block);
      return;
    }
  auto emag = [&] (int i) {
    if (i > 0 && i < int (in_block.env.size()))
      return in_block.env_f (i);
    return 0.0f;
  };
  auto emag_inter = [&] (float p) {
    int ip = p;
    float frac = p - ip;
    return emag (ip) * (1 - frac) + emag (ip + 1) * frac;
  };
  auto set_mags = [&] (float *mags, size_t mags_count) {
    /* compute energy before formant correction */
    float e1 = 0;
    for (size_t i = 0; i < in_block.mags.size(); i++)
      {
        float mag = in_block.mags_f (i);
        e1 += mag * mag;
      }
    /* compute energy after formant correction */
    float e2 = 0;
    for (size_t i = 0; i < mags_count; i++)
      {
        float mag = mags[i];
        e2 += mag * mag;
      }

    const float threshold = 1e-9;
    float norm = (e2 > threshold) ? sqrt (e1 / e2) : 1;

    /* generate normalized block mags */
    assert (out_block.freqs.size() == mags_count);
    for (size_t i = 0; i < mags_count; i++)
      mags[i] *= norm;
    uint16_t imags[mags_count];
    sm_factor2idbs (mags, mags_count, imags);
    out_block.mags.assign (imags, imags + mags_count);
  };
  out_block.noise.assign (in_block.noise);
  if (mode == MODE_PRESERVE_SPECTRAL_ENVELOPE)
    {
      out_block.freqs.set_capacity (in_block.freqs.size());
      const float e_tune_factor = 1 / in_block.env_f0;
      float mags[in_block.freqs.size()];
      size_t count = 0;

      for (size_t i = 0; i < in_block.freqs.size(); i++)
        {
          float freq = in_block.freqs_f (i) * e_tune_factor;

          if (freq > 0.5)
            {
              float old_env_mag = emag_inter (freq);
              float new_env_mag = emag_inter (freq * ratio);

              out_block.freqs.push_back (in_block.freqs[i]);
              mags[count++] = in_block.mags_f (i) / old_env_mag * new_env_mag;;
            }

          if (freq * ratio > max_partials)
            break;
        }
      set_mags (mags, count);
    }
  else if (mode == MODE_HARMONIC_RESYNTHESIS)
    {
      int partials = std::min (sm_round_positive (max_partials / ratio) + 1, RESYNTH_MAX_PARTIALS);
      float freqs[partials];
      uint16_t ifreqs[partials];
      float mags[partials];
      size_t mags_count = 0;

      float ff = in_block.env_f0;
      if (fuzzy_frac > 1)
        {
          detune_factors.swap (next_detune_factors);
          next_detune_factors.clear();
          fuzzy_frac -= int (fuzzy_frac);
          fuzzy_resynth_freq = detune_random.random_double_range (RESYNTH_MIN_FREQ, RESYNTH_MAX_FREQ);
        }

      gen_detune_factors (detune_factors, partials);
      gen_detune_factors (next_detune_factors, partials);

      for (int i = 1; i < partials; i++)
        {
          freqs[mags_count] = i * ff * (detune_factors[i] * (1 - fuzzy_frac) + next_detune_factors[i] * fuzzy_frac);
          mags[mags_count++] = emag_inter (i * ratio);
        }
      sm_freq2ifreqs (freqs, mags_count, ifreqs);
      out_block.freqs.assign (ifreqs, ifreqs + mags_count);
      set_mags (mags, mags_count);
    }
  else
    {
      g_assert_not_reached();
    }
}


