// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_WAVSETBUILDER_HH
#define SPECTMORPH_WAVSETBUILDER_HH

#include "sminstrument.hh"
#include "smwavset.hh"
#include "sminstenccache.hh"

namespace SpectMorph
{

class WavSetBuilder
{
  struct SampleData
  {
    int          midi_note;
    double       volume;
    Sample::Loop loop;
    double       clip_start_ms;
    double       clip_end_ms;
    double       loop_start_ms;
    double       loop_end_ms;

    Sample::SharedP shared;
  };
  std::vector<SampleData> sample_data_vec;
  WavSet *wav_set;
  InstEncCache::Group       *cache_group = nullptr;

  std::function<bool()>      kill_function;
  bool killed();

  double                     global_volume = 0;
  Instrument::AutoVolume     auto_volume;
  Instrument::AutoTune       auto_tune;
  Instrument::EncoderConfig  encoder_config;
  bool keep_samples;

  void apply_loop_settings();
  void apply_volume_settings();
  void apply_auto_volume();
  void apply_auto_tune();

  void add_sample (const Sample *sample);
public:
  WavSetBuilder (const Instrument *instrument, bool keep_samples);
  ~WavSetBuilder();

  void set_kill_function (const std::function<bool()>& kill_function);
  void set_cache_group (InstEncCache::Group *group);
  WavSet *run();
};

}

#endif
