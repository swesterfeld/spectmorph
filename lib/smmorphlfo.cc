// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphlfo.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

static LeakDebugger leak_debugger ("SpectMorph::MorphLFO");

MorphLFOProperties::MorphLFOProperties (MorphLFO *lfo) :
  frequency (lfo, "Frequency", "%.3f Hz", 0.01, 10, &MorphLFO::frequency, &MorphLFO::set_frequency),
  depth (lfo, "Depth", "-", 0, 1, &MorphLFO::depth, &MorphLFO::set_depth),
  center (lfo, "Center", "%.2f", -1, 1, &MorphLFO::center, &MorphLFO::set_center),
  start_phase (lfo, "Start Phase", "%.1f", -180, 180, &MorphLFO::start_phase, &MorphLFO::set_start_phase)
{
  /* FIXME: ideally the storage format should be changed -> store depth as percent */
  depth.set_custom_formatter ([](float f) -> string { return string_locale_printf ("%.1f %%", f * 100); });
}

MorphLFO::MorphLFO (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  m_config.wave_type = WAVE_SINE;
  m_config.frequency = 1;
  m_config.depth = 1;
  m_config.center = 0;
  m_config.start_phase = 0;
  m_config.sync_voices = false;
  m_config.beat_sync = false;
  m_config.note = NOTE_1_4;
  m_config.note_mode = NOTE_MODE_STRAIGHT;

  leak_debugger.add (this);
}

MorphLFO::~MorphLFO()
{
  leak_debugger.del (this);
}

const char *
MorphLFO::type()
{
  return "SpectMorph::MorphLFO";
}

int
MorphLFO::insert_order()
{
  return 200;
}

bool
MorphLFO::save (OutFile& out_file)
{
  out_file.write_int ("wave_type", m_config.wave_type);
  out_file.write_float ("frequency", m_config.frequency);
  out_file.write_float ("depth", m_config.depth);
  out_file.write_float ("center", m_config.center);
  out_file.write_float ("start_phase", m_config.start_phase);
  out_file.write_bool ("sync_voices", m_config.sync_voices);
  out_file.write_bool ("beat_sync", m_config.beat_sync);
  out_file.write_int ("note", m_config.note);
  out_file.write_int ("note_mode", m_config.note_mode);

  return true;
}

bool
MorphLFO::load (InFile& ifile)
{
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::INT)
        {
          if (ifile.event_name() == "wave_type")
            {
              m_config.wave_type = static_cast<WaveType> (ifile.event_int());
            }
          else if (ifile.event_name() == "note")
            {
              m_config.note = static_cast<Note> (ifile.event_int());
            }
          else if (ifile.event_name() == "note_mode")
            {
              m_config.note_mode = static_cast<NoteMode> (ifile.event_int());
            }
          else
            {
              g_printerr ("bad int\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::FLOAT)
        {
          if (ifile.event_name() == "frequency")
            {
              m_config.frequency = ifile.event_float();
            }
          else if (ifile.event_name() == "depth")
            {
              m_config.depth = ifile.event_float();
            }
          else if (ifile.event_name() == "center")
            {
              m_config.center = ifile.event_float();
            }
          else if (ifile.event_name() == "start_phase")
            {
              m_config.start_phase = ifile.event_float();
            }
          else
            {
              g_printerr ("bad float\n");
              return false;
            }
        }
      else if (ifile.event() == InFile::BOOL)
        {
          if (ifile.event_name() == "sync_voices")
            {
              m_config.sync_voices = ifile.event_bool();
            }
          else if (ifile.event_name() == "beat_sync")
            {
              m_config.beat_sync = ifile.event_bool();
            }
          else
            {
              g_printerr ("bad bool\n");
              return false;
            }
        }
      else
        {
          g_printerr ("bad event\n");
          return false;
        }
      ifile.next_event();
    }
  return true;
}

MorphOperator::OutputType
MorphLFO::output_type()
{
  return OUTPUT_CONTROL;
}

MorphLFO::WaveType
MorphLFO::wave_type()
{
  return m_config.wave_type;
}

void
MorphLFO::set_wave_type (WaveType new_wave_type)
{
  m_config.wave_type = new_wave_type;

  m_morph_plan->emit_plan_changed();
}

float
MorphLFO::frequency() const
{
  return m_config.frequency;
}

void
MorphLFO::set_frequency (float frequency)
{
  m_config.frequency = frequency;

  m_morph_plan->emit_plan_changed();
}

float
MorphLFO::depth() const
{
  return m_config.depth;
}

void
MorphLFO::set_depth (float depth)
{
  m_config.depth = depth;

  m_morph_plan->emit_plan_changed();
}

float
MorphLFO::center() const
{
  return m_config.center;
}

void
MorphLFO::set_center (float center)
{
  m_config.center = center;

  m_morph_plan->emit_plan_changed();
}

float
MorphLFO::start_phase() const
{
  return m_config.start_phase;
}

void
MorphLFO::set_start_phase (float start_phase)
{
  m_config.start_phase = start_phase;

  m_morph_plan->emit_plan_changed();
}

bool
MorphLFO::sync_voices() const
{
  return m_config.sync_voices;
}

void
MorphLFO::set_sync_voices (float sync_voices)
{
  m_config.sync_voices = sync_voices;

  m_morph_plan->emit_plan_changed();
}

bool
MorphLFO::beat_sync() const
{
  return m_config.beat_sync;
}

void
MorphLFO::set_beat_sync (bool beat_sync)
{
  m_config.beat_sync = beat_sync;

  m_morph_plan->emit_plan_changed();
}

MorphLFO::Note
MorphLFO::note() const
{
  return m_config.note;
}

void
MorphLFO::set_note (Note note)
{
  m_config.note = note;

  m_morph_plan->emit_plan_changed();
}

MorphLFO::NoteMode
MorphLFO::note_mode() const
{
  return m_config.note_mode;
}

void
MorphLFO::set_note_mode (NoteMode mode)
{
  m_config.note_mode = mode;

  m_morph_plan->emit_plan_changed();
}

MorphOperatorConfig *
MorphLFO::clone_config()
{
  return new Config (m_config);
}
