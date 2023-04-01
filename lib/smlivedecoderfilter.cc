// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smlivedecoderfilter.hh"
#include "smmorphoutputmodule.hh"

using namespace SpectMorph;

using std::max;
using std::min;

void
LiveDecoderFilter::retrigger (float note)
{
  ladder_filter.reset();
  sk_filter.reset();

  envelope.set_rate (mix_freq);
  envelope.start();

  smooth_first = true;
  current_note = note;
}

void
LiveDecoderFilter::release()
{
  envelope.stop();
}

/* FIXME: FILTER: dedup */
static float
exp_percent (float p, float min_out, float max_out, float slope)
{
  /* exponential curve from 0 to 1 with configurable slope */
  const double x = (pow (2, (p / 100.) * slope) - 1) / (pow (2, slope) - 1);

  /* rescale to interval [min_out, max_out] */
  return x * (max_out - min_out) + min_out;
}

/* FIXME: FILTER: dedup */
static float
xparam_percent (float p, float min_out, float max_out, float slope)
{
  /* rescale xparam function to interval [min_out, max_out] */
  return sm_xparam (p / 100.0, slope) * (max_out - min_out) + min_out;
}

void
LiveDecoderFilter::set_config (MorphOutputModule *output_module, const MorphOutput::Config *cfg, float mix_freq)
{
  this->output_module = output_module;
  this->mix_freq = mix_freq;

  filter_type = cfg->filter_type;
  key_tracking = cfg->filter_key_tracking;

  float attack  = xparam_percent (cfg->filter_attack, 2, 5000, 3) / 1000;
  float decay   = xparam_percent (cfg->filter_decay, 2, 5000, 3) / 1000;
  float release = exp_percent (cfg->filter_release, 2, 200, 3) / 1000; /* FIXME: FILTER: this may not be the best solution */
  float sustain = cfg->filter_sustain;
  if (0)
    {
      printf ("%.2f ms -  %.2f ms  -  %.2f ms\n",
          attack * 1000,
          decay * 1000,
          release * 1000);
    }

  envelope.set_shape (FlexADSR::Shape::EXPONENTIAL);
  envelope.set_attack (attack);
  envelope.set_decay (decay);
  envelope.set_sustain (sustain);
  envelope.set_release (release);
  depth_octaves = cfg->filter_depth / 12;

  switch (cfg->filter_ladder_mode)
    {
      case MorphOutput::FILTER_LADDER_LP1: ladder_filter.set_mode (LadderVCF::LP1); break;
      case MorphOutput::FILTER_LADDER_LP2: ladder_filter.set_mode (LadderVCF::LP2); break;
      case MorphOutput::FILTER_LADDER_LP3: ladder_filter.set_mode (LadderVCF::LP3); break;
      case MorphOutput::FILTER_LADDER_LP4: ladder_filter.set_mode (LadderVCF::LP4); break;
    }
  switch (cfg->filter_sk_mode)
    {
      case MorphOutput::FILTER_SK_LP1: sk_filter.set_mode (SKFilter::LP1); break;
      case MorphOutput::FILTER_SK_LP2: sk_filter.set_mode (SKFilter::LP2); break;
      case MorphOutput::FILTER_SK_LP3: sk_filter.set_mode (SKFilter::LP3); break;
      case MorphOutput::FILTER_SK_LP4: sk_filter.set_mode (SKFilter::LP4); break;
      case MorphOutput::FILTER_SK_LP6: sk_filter.set_mode (SKFilter::LP6); break;
      case MorphOutput::FILTER_SK_LP8: sk_filter.set_mode (SKFilter::LP8); break;
      case MorphOutput::FILTER_SK_BP2: sk_filter.set_mode (SKFilter::BP2); break;
      case MorphOutput::FILTER_SK_BP4: sk_filter.set_mode (SKFilter::BP4); break;
      case MorphOutput::FILTER_SK_BP6: sk_filter.set_mode (SKFilter::BP6); break;
      case MorphOutput::FILTER_SK_BP8: sk_filter.set_mode (SKFilter::BP8); break;
      case MorphOutput::FILTER_SK_HP1: sk_filter.set_mode (SKFilter::HP1); break;
      case MorphOutput::FILTER_SK_HP2: sk_filter.set_mode (SKFilter::HP2); break;
      case MorphOutput::FILTER_SK_HP3: sk_filter.set_mode (SKFilter::HP3); break;
      case MorphOutput::FILTER_SK_HP4: sk_filter.set_mode (SKFilter::HP4); break;
      case MorphOutput::FILTER_SK_HP6: sk_filter.set_mode (SKFilter::HP6); break;
      case MorphOutput::FILTER_SK_HP8: sk_filter.set_mode (SKFilter::HP8); break;
    }
}

void
LiveDecoderFilter::process (size_t n_values, float *audio)
{
  if (!n_values)
    return;

  auto start_smoothing = [&] (SmoothValue& smooth_value, float new_value, float speed_ms) {
    float abs_diff = std::abs (smooth_value.value - new_value);

    if (smooth_first || abs_diff < 1e-5)
      {
        smooth_value.value = new_value;
        smooth_value.delta = 0;
        smooth_value.constant = true;
      }
    else
      {
        int min_steps = abs_diff * mix_freq * 0.001f * speed_ms;
        int steps = max<int> (min_steps, n_values);
        smooth_value.delta = (new_value - smooth_value.value) / steps;
        smooth_value.constant = false;
      }
  };

  float delta_cent = (current_note - 60) * key_tracking;
  float filter_keytrack_octaves = delta_cent * (1 / 1200.f);

  /* setup cutoff smoothing: at least 0.5ms per octave */
  float new_log_cutoff = log2f (output_module->filter_cutoff_mod()) + filter_keytrack_octaves;
  start_smoothing (log_cutoff_smooth, new_log_cutoff, 0.5f);

  /* setup reso smoothing: we want at least 5ms for the whole resonance range */
  float new_resonance = output_module->filter_resonance_mod() * 0.01f;
  start_smoothing (resonance_smooth, new_resonance, 5.0f);

  /* setup drive smoothing: we want 10ms for 36 dB */
  float new_drive = output_module->filter_drive_mod();
  start_smoothing (drive_smooth, new_drive, 10.f / 36);

  smooth_first = false;

  auto filter_process_block = [&] (auto& filter)
    {
      auto gen_filter_input = [&] (float *freq_in, float *reso_in, float *drive_in, uint count)
        {
          envelope.process (freq_in, count);
          for (uint i = 0; i < count; i++)
            {
              log_cutoff_smooth.value += log_cutoff_smooth.delta;
              resonance_smooth.value += resonance_smooth.delta;
              drive_smooth.value += drive_smooth.delta;

              freq_in[i] = exp2f (log_cutoff_smooth.value + freq_in[i] * depth_octaves);
              reso_in[i] = resonance_smooth.value;
              drive_in[i] = drive_smooth.value;
            }
        };
      const bool const_freq = log_cutoff_smooth.constant && false; //envelope.is_constant();
      const bool const_reso = resonance_smooth.constant;
      const bool const_drive = drive_smooth.constant;

      if (const_freq && const_reso && const_drive)
        {
          /* use more efficient version of the filter computation if all parameters are constants */
          float freq, reso, drive;
          gen_filter_input (&freq, &reso, &drive, 1);

          filter.set_freq (freq);
          filter.set_reso (reso);
          filter.set_drive (drive);
          filter.process_block (n_values, audio);
        }
      else
        {
          /* generic version: pass per-sample values for freq, reso and drive */
          float freq_in[n_values], reso_in[n_values], drive_in[n_values];
          gen_filter_input (freq_in, reso_in, drive_in, n_values);

          filter.process_block (n_values, audio, nullptr, freq_in, reso_in, drive_in);
        }
    };

  if (filter_type == MorphOutput::FILTER_TYPE_LADDER)
    filter_process_block (ladder_filter);
  else
    filter_process_block (sk_filter);
}

int
LiveDecoderFilter::idelay()
{
  switch (filter_type)
    {
      case MorphOutput::FILTER_TYPE_LADDER:     return ladder_filter.delay();
      case MorphOutput::FILTER_TYPE_SALLEN_KEY: return sk_filter.delay();
    }
  g_assert_not_reached();
}
