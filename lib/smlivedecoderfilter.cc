// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smlivedecoderfilter.hh"
#include "smmorphoutputmodule.hh"

using namespace SpectMorph;

void
LiveDecoderFilter::retrigger (float note)
{
  ladder_filter.reset();
  sk_filter.reset();

  smooth_first = true;
  current_note = note;
}

void
LiveDecoderFilter::set_config (MorphOutputModule *output_module, const MorphOutput::Config *cfg, float mix_freq)
{
  this->output_module = output_module;

  cutoff_smooth.reset (mix_freq, 0.010);
  resonance_smooth.reset (mix_freq, 0.010);
  drive_smooth.reset (mix_freq, 0.010);

  filter_type = cfg->filter_type;
  key_tracking = cfg->filter_key_tracking;

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
  float delta_cent = (current_note - 60) * key_tracking;
  float filter_keytrack_factor = exp2f (delta_cent * (1 / 1200.f));

  cutoff_smooth.set (output_module->filter_cutoff_mod() * filter_keytrack_factor, smooth_first);
  resonance_smooth.set (output_module->filter_resonance_mod() * 0.01, smooth_first);
  drive_smooth.set (output_module->filter_drive_mod(), smooth_first);

  smooth_first = false;

  auto filter_process_block = [&] (auto& filter)
    {
      float freq_in[n_values];
      float reso_in[n_values];
      float drive_in[n_values];

      for (uint i = 0; i < n_values; i++)
        {
          freq_in[i] = cutoff_smooth.get_next(); // FIXME * exp2f (filter_envelope.get_next() * filter_depth_octaves);
          reso_in[i] = resonance_smooth.get_next();
          drive_in[i] = drive_smooth.get_next();
        }
      filter.process_block (n_values, audio, nullptr, freq_in, reso_in, drive_in);
    };

  if (filter_type == MorphOutput::FILTER_TYPE_LADDER)
    filter_process_block (ladder_filter);
  else
    filter_process_block (sk_filter);
}
