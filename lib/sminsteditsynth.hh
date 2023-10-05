// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_INST_EDIT_SYNTH_HH
#define SPECTMORPH_INST_EDIT_SYNTH_HH

#include "smlivedecoder.hh"
#include "smnotifybuffer.hh"

#include <string>
#include <memory>

namespace SpectMorph {

struct MidiSynthCallbacks;

class InstEditSynth
{
public:
  struct Decoders {
    std::unique_ptr<WavSet> wav_set;
    std::unique_ptr<WavSet> ref_wav_set;
    std::vector<std::unique_ptr<LiveDecoder>> decoders;
  };
private:
  enum class State {
    ON,
    RELEASE,
    IDLE
  };
  struct Voice {
    State                      state = State::IDLE;
    LiveDecoder               *decoder = nullptr;
    double                     decoder_factor = 0;
    int                        note = 0;
    unsigned int               layer = 0;
    int                        channel = 0;
    int                        clap_id = -1;
  };

  static constexpr uint        n_layers = 3;
  static constexpr uint        voices_per_layer = 64;

  float                        mix_freq;
  std::vector<Voice>           voices;
  Decoders                     decoders;

public:
  InstEditSynth (float mix_freq);
  ~InstEditSynth();

  Decoders create_decoders (WavSet *take_wav_set, WavSet *take_ref_wav_set);
  void swap_decoders (Decoders& decoders);

  void process_note_on (int channel, int note, int clap_id, unsigned int layer);
  void process_note_off (int channel, int note, unsigned int layer);

  void process (float *output, size_t n_values, RTMemoryArea& rt_memory_area, NotifyBuffer& notify_buffer, MidiSynthCallbacks *process_callbacks);
};

}

#endif
