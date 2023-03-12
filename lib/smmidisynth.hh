// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MIDI_SYNTH_HH
#define SPECTMORPH_MIDI_SYNTH_HH

#include "smmorphplansynth.hh"
#include "sminsteditsynth.hh"
#include "smnotifybuffer.hh"

namespace SpectMorph {

class MidiSynth
{
public:
  struct TerminatedVoice
  {
    int key;
    int channel;
    int clap_id;
  };
  struct ProcessCallbacks
  {
    virtual void terminated_voice (TerminatedVoice& voice) = 0;
  };

private:
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
    int          clap_id;

    std::array<float, MorphPlan::N_CONTROL_INPUTS> modulation;

    Voice() :
      mp_voice (NULL),
      state (STATE_IDLE),
      pedal (false)
    {
      modulation.fill (0);
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
  NotifyBuffer          m_notify_buffer;
  ProcessCallbacks     *m_process_callbacks = nullptr;

  std::vector<float>    control = std::vector<float> (MorphPlan::N_CONTROL_INPUTS);

  Voice  *alloc_voice();
  void    free_unused_voices();
  bool    update_mono_voice();
  float   freq_from_note (float note);
  void    notify_active_voice_status();

  void set_mono_enabled (bool new_value);
  void process_audio (const TimeInfo& block_time, float *output, size_t n_values);
  void process_note_on (const TimeInfo& block_time, int channel, int midi_note, int midi_velocity, int clap_id);
  void process_note_off (int midi_note);
  void process_midi_controller (int controller, int value);
  void process_pitch_bend (int channel, double semi_tones);
  void start_pitch_bend (Voice *voice, double dest_freq, double time_ms);
  void kill_all_active_voices();

  enum MidiEventType {
    EVENT_NOTE_ON,
    EVENT_CONTROL_VALUE,
    EVENT_MOD_VALUE,
    EVENT_MIDI
  };

  struct MidiEvent
  {
    MidiEventType type = EVENT_MIDI;

    unsigned int  offset;

    /* this block is only used if event_type == EVENT_NOTE_ON */
    int           clap_id;
    int           xchannel;
    int           key = -1;
    double        velocity;

    /* this block is only used if event_type == EVENT_CONTROL_VALUE || event_type == EVENT_MOD_VALUE */
    int           control_input = -1;
    float         value;

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
  void process (float *output, size_t n_values, ProcessCallbacks *process_callbacks = nullptr);

  void add_note_on_event (uint offset, int clap_id, int channel, int key, double velocity);
  void add_control_input_event (uint offset, int i, float value);

  void set_control_input (int i, float value);
  void add_modulation_event (uint offset, int i, float value);
  void add_modulation_clap_id_event (uint offset, int i, float value, int clap_id);
  void add_modulation_key_event (uint offset, int i, float value, int key, int channel);

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
  NotifyBuffer *notify_buffer();
};

class SynthNotifyEvent
{
public:
  virtual
  ~SynthNotifyEvent()
  {
  }
  static SynthNotifyEvent *
  create (NotifyBuffer& buffer);
};

enum NotifyEventType
{
  INST_EDIT_VOICE_EVENT = 748293, // some random number
  VOICE_OP_VALUES_EVENT,
  ACTIVE_VOICE_STATUS_EVENT
};

struct InstEditVoiceEvent : public SynthNotifyEvent
{
  struct Voice
  {
    int   note;
    int   layer;
    float current_pos;
    float fundamental_note;
  };
  InstEditVoiceEvent (NotifyBuffer& buffer) :
    voices (buffer.read_seq<Voice>())
  {
  }
  std::vector<Voice> voices;
};

struct VoiceOpValuesEvent : public SynthNotifyEvent
{
  struct Voice
  {
    uintptr_t voice;
    uintptr_t op;
    float value;
  };
  VoiceOpValuesEvent (NotifyBuffer& buffer) :
    voices (buffer.read_seq<Voice>())
  {
  }
  std::vector<Voice> voices;
};

struct ActiveVoiceStatusEvent : public SynthNotifyEvent
{
  ActiveVoiceStatusEvent (NotifyBuffer& buffer) :
    voice (buffer.read_seq<uintptr_t>())
  {
    for (auto& ctrl : control)
      ctrl = buffer.read_seq<float>();
  }
  std::vector<uintptr_t> voice;
  std::vector<float>     control[MorphPlan::N_CONTROL_INPUTS];
};

}

#endif /* SPECTMORPH_MIDI_SYNTH_HH */
