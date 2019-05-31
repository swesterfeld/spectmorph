// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INST_EDIT_SYNTH_HH
#define SPECTMORPH_INST_EDIT_SYNTH_HH

#include "smlivedecoder.hh"
#include "smbinbuffer.hh"

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
    unsigned int                 layer = 0;
  };

  static constexpr uint        n_layers = 3;

  float                        mix_freq;
  std::unique_ptr<WavSet>      wav_set;
  std::unique_ptr<WavSet>      ref_wav_set;
  std::vector<Voice>           voices;
  BinBuffer                    notify_buffer;

  std::vector<std::string>     out_events;
public:
  InstEditSynth (float mix_freq);
  ~InstEditSynth();

  void take_wav_sets (WavSet *new_wav_set, WavSet *new_ref_wav_set);

  void handle_midi_event (const unsigned char *midi_data, unsigned int layer);
  void process (float *output, size_t n_values);

  std::vector<std::string> take_out_events();
};

}

#endif
