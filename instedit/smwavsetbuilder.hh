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
    double       loop_start_ms;
    double       loop_end_ms;
  };
  std::vector<SampleData> sample_data_vec;

  void apply_loop_settings();
  void add_sample (const Sample *sample);
public:
  WavSetBuilder (const Instrument *instrument);

  void run();

  void get_result (WavSet& wav_set);
};

}

#endif
