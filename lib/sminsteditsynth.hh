// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INST_EDIT_SYNTH_HH
#define SPECTMORPH_INST_EDIT_SYNTH_HH

#include "smlivedecoder.hh"

#include <string>
#include <memory>

namespace SpectMorph {

class InstEditSynth
{
  enum class State {
    ON,
    RELEASE,
    IDLE
  };
  struct Voice {
    State                        state = State::IDLE;
    std::unique_ptr<LiveDecoder> decoder;
    double                       decoder_factor = 0;
    unsigned int                 note = 0;
  };

  float                        mix_freq;
  std::unique_ptr<WavSet>      wav_set;
  std::vector<Voice>           voices;

public:
  InstEditSynth (float mix_freq);
  ~InstEditSynth();

  void take_wav_set (WavSet *new_wav_set, bool enable_original_samples);

  void handle_midi_event (const unsigned char *midi_data);
  void process (float *output, size_t n_values);

  double current_pos();
};

}

#endif
