// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WAVSETBUILDER_HH
#define SPECTMORPH_WAVSETBUILDER_HH

#include "sminstrument.hh"
#include "smwavset.hh"

namespace SpectMorph
{

class WavSetBuilder
{
  struct SampleData
  {
    int       midi_note;
    WavData  *wav_data_ptr;  // FIXME: clean this up

    Sample::Loop loop;
    double       clip_start_ms;
    double       clip_end_ms;
    double       loop_start_ms;
    double       loop_end_ms;
  };
  std::vector<SampleData> sample_data_vec;
  WavSet *wav_set;
  std::string name;

  Instrument::AutoVolume auto_volume;
  Instrument::AutoTune   auto_tune;
  bool keep_samples;

  void apply_loop_settings();
  void apply_auto_volume();
  void apply_auto_tune();

  void add_sample (const Sample *sample);
public:
  WavSetBuilder (const Instrument *instrument, bool keep_samples);
  ~WavSetBuilder();

  WavSet *run();
};

}

#endif
