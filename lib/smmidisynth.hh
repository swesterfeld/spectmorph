// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MIDI_SYNTH_HH
#define SPECTMORPH_MIDI_SYNTH_HH

#include "smmorphplansynth.hh"
#include "smnotifybuffer.hh"
#include "sminsteditsynth.hh"
#include "smrtmemory.hh"

#include <array>

namespace SpectMorph {

struct MidiSynthCallbacks
{
  struct TerminatedVoice
  {
    int key;
    int channel;
    int clap_id;
  };
  virtual void terminated_voice (TerminatedVoice& voice) = 0;
};

class MidiSynth
{
private:
  enum EventType {
    EVENT_NOTE_ON,
    EVENT_NOTE_OFF,
    EVENT_CONTROL_VALUE,
    EVENT_MOD_VALUE,
    EVENT_PITCH_EXPRESSION,
    EVENT_PITCH_BEND,
    EVENT_CC
  };

  struct NoteEvent
  {
    int   clap_id;
    int   channel;
    int   key;
    float velocity;
  };
  struct ExpressionEvent
  {
    int   channel;
    int   key;
    float value;
  };
  struct ValueEvent
  {
    int   control_input;
    float value;
  };
  struct ModValueEvent
  {
    int   clap_id;
    int   channel;
    int   key;
    int   control_input;
    float value;
  };
  struct PitchBendEvent
  {
    int   channel;
    float value;
  };
  struct CCEvent
  {
    int   controller;
    int   value;
  };
  struct Event
  {
    EventType         type;
    unsigned int      offset;

    union {
      NoteEvent       note;       // EVENT_NOTE_ON, EVENT_NOTE_OFF
      ExpressionEvent expr;       // EVENT_PITCH_EXPRESSION
      ValueEvent      value;      // EVENT_CONTROL_VALUE
      ModValueEvent   mod;        // EVENT_MOD_VALUE
      PitchBendEvent  pitch_bend; // EVENT_PITCH_BEND
      CCEvent         cc;         // EVENT_CC
    };
  };
  std::vector<Event>  events;

  typedef std::array<float, MorphPlan::N_CONTROL_INPUTS> ModArray;

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

    ModArray     modulation;

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

  constexpr static int  MAX_VOICES = 256;

  MorphPlanSynth        morph_plan_synth;
  InstEditSynth         m_inst_edit_synth;

  std::vector<Voice>    voices;
  std::vector<Voice *>  idle_voices;
  std::vector<Voice *>  active_voices;
  ModArray              global_modulation;
  double                m_mix_freq;
  double                m_gain = 1;
  double                m_tempo = 120;
  double                m_ppq_pos = 0;
  TimeInfoGenerator     m_time_info_gen;
  bool                  pedal_down;
  uint64                audio_time_stamp;
  bool                  mono_enabled;
  float                 portamento_glide;
  int                   portamento_note_id;
  int                   next_note_id;
  bool                  inst_edit = false;
  bool                  m_control_by_cc = false;
  RTMemoryArea          m_rt_memory_area;
  NotifyBuffer          m_notify_buffer;
  MidiSynthCallbacks   *m_process_callbacks = nullptr;

  std::vector<float>    control = std::vector<float> (MorphPlan::N_CONTROL_INPUTS);

  Voice  *alloc_voice();
  void    free_unused_voices();
  bool    update_mono_voice();
  float   freq_from_note (float note);
  void    notify_active_voice_status();

  void set_mono_enabled (bool new_value);
  void process_audio (float *output, size_t n_values);
  void process_note_on (const NoteEvent& note);
  void process_note_off (int channel, int midi_note);
  void process_midi_controller (int controller, int value);
  void process_pitch_bend (int channel, double semi_tones);
  void process_mod_value (const ModValueEvent& mod);
  void start_pitch_bend (Voice *voice, double dest_freq, double time_ms);
  void kill_all_active_voices();

public:
  MidiSynth (double mix_freq, size_t n_voices);

  void add_midi_event (size_t offset, const unsigned char *midi_data);
  void process (float *output, size_t n_values, MidiSynthCallbacks *process_callbacks = nullptr);

  void add_note_on_event (uint offset, int clap_id, int channel, int key, float velocity);
  void add_note_off_event (uint offset, int channel, int key);
  void add_control_input_event (uint offset, int i, float value);
  void add_pitch_expression_event (uint offset, float value, int channel, int key);
  void add_modulation_event (uint offset, int i, float value, int clap_id, int channel, int key);

  void set_control_input (int i, float value);

  void set_tempo (double tempo);
  void set_ppq_pos (double ppq_pos);
  MorphPlanSynth::UpdateP prepare_update (const MorphPlan& plan);
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
    voice (buffer.read_seq<uintptr_t>()),
    velocity (buffer.read_seq<float>())
  {
    for (auto& ctrl : control)
      ctrl = buffer.read_seq<float>();
  }
  std::vector<uintptr_t> voice;
  std::vector<float>     velocity;
  std::vector<float>     control[MorphPlan::N_CONTROL_INPUTS];
};

}

#endif /* SPECTMORPH_MIDI_SYNTH_HH */
