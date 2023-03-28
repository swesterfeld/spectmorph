// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smlivedecoderfilter.hh"

using namespace SpectMorph;

void
LiveDecoderFilter::reset()
{
  ladder_filter.reset();
  sk_filter.reset();
}

void
LiveDecoderFilter::set_config (const MorphOutput::Config *cfg, float mix_freq)
{
  filter_type = cfg->filter_type;

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
  float freq = 1000;
  float reso = 0.8;
  float drive = 0;
  auto filter_process_block = [&] (auto& filter)
    {
      filter.set_freq (freq);
      filter.set_reso (reso);
      filter.set_drive (drive);
      filter.process_block (n_values, audio);
    };

  if (filter_type == MorphOutput::FILTER_TYPE_LADDER)
    filter_process_block (ladder_filter);
  else
    filter_process_block (sk_filter);
}
