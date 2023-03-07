// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MIDI_SYNTH_HH
#define SPECTMORPH_MIDI_SYNTH_HH

#include "smmorphplansynth.hh"
#include "sminsteditsynth.hh"

namespace SpectMorph {

class MidiSynth
{
  class Voice
  {
  public:
    enum State {
      STATE_IDLE,
      STATE_ON,
      STATE_RELEASE
    };
    enum class MonoType {
      POLY,
      MONO,
      SHADOW
    };
    MorphPlanVoice *mp_voice;

    State        state;
    MonoType     mono_type;
    bool         pedal;
    int          midi_note;
    int          channel;
    double       gain;
    double       freq;
    double       pitch_bend_freq;
    double       pitch_bend_factor;
    int          pitch_bend_steps;
    int          note_id;

    Voice() :
      mp_voice (NULL),
      state (STATE_IDLE),
      pedal (false)
    {
    }
    ~Voice()
    {
      mp_voice = NULL;
    }
  };

  MorphPlanSynth        morph_plan_synth;
  InstEditSynth         m_inst_edit_synth;

  std::vector<Voice>    voices;
  std::vector<Voice *>  idle_voices;
  std::vector<Voice *>  active_voices;
  double                m_mix_freq;
  double                m_gain = 1;
  double                m_tempo = 120;
  double                m_ppq_pos = 0;
  bool                  pedal_down;
  uint64                audio_time_stamp;
  bool                  mono_enabled;
  float                 portamento_glide;
  int                   portamento_note_id;
  int                   next_note_id;
  bool                  inst_edit = false;
  bool                  m_control_by_cc = false;
  BinBuffer             notify_buffer;
  std::vector<std::string> out_events;

  std::vector<float>    control = std::vector<float> (MorphPlan::N_CONTROL_INPUTS);

  Voice  *alloc_voice();
  void    free_unused_voices();
  bool    update_mono_voice();
  float   freq_from_note (float note);
  void    notify_active_voice_status();

  void set_mono_enabled (bool new_value);
  void process_audio (const TimeInfo& block_time, float *output, size_t n_values);
  void process_note_on (const TimeInfo& block_time, int channel, int midi_note, int midi_velocity);
  void process_note_off (int midi_note);
  void process_midi_controller (int controller, int value);
  void process_pitch_bend (int channel, double semi_tones);
  void start_pitch_bend (Voice *voice, double dest_freq, double time_ms);
  void kill_all_active_voices();

  struct MidiEvent
  {
    unsigned int  offset;
    char          midi_data[3];

    bool is_note_on() const;
    bool is_note_off() const;
    bool is_controller() const;
    bool is_pitch_bend() const;
    int  channel() const;
  };
  std::vector<MidiEvent>  midi_events;

public:
  MidiSynth (double mix_freq, size_t n_voices);

  void add_midi_event (size_t offset, const unsigned char *midi_data);
  void process (float *output, size_t n_values);

  void set_control_input (int i, float value);
  void set_tempo (double tempo);
  void set_ppq_pos (double ppq_pos);
  MorphPlanSynth::UpdateP prepare_update (MorphPlanPtr plan);
  void apply_update (MorphPlanSynth::UpdateP update);
  double mix_freq() const;

  size_t active_voice_count() const;

  void set_inst_edit (bool inst_edit);
  void set_gain (double gain);
  void set_control_by_cc (bool control_by_cc);
  InstEditSynth *inst_edit_synth();

  std::vector<std::string> take_out_events();
};

class SynthNotifyEvent
{
public:
  virtual
  ~SynthNotifyEvent()
  {
  }
  static SynthNotifyEvent *
  create (const std::string& str);
};

struct InstEditVoiceEvent : public SynthNotifyEvent
{
  std::vector<int>   note;
  std::vector<int>   layer;
  std::vector<float> current_pos;
  std::vector<float> fundamental_note;
};

struct VoiceOpValuesEvent : public SynthNotifyEvent
{
  std::vector<uintptr_t> voice;
  std::vector<uintptr_t> op;
  std::vector<float>     value;
};

struct ActiveVoiceStatusEvent : public SynthNotifyEvent
{
  std::vector<uintptr_t> voice;
  std::vector<float>     control[MorphPlan::N_CONTROL_INPUTS];
};

}

#endif /* SPECTMORPH_MIDI_SYNTH_HH */
