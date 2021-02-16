// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphlfo.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

static LeakDebugger leak_debugger ("SpectMorph::MorphLFO");

MorphLFO::MorphLFO (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  EnumInfo wave_type_enum_info (
    {
      { MorphLFO::WAVE_SINE,           "Sine" },
      { MorphLFO::WAVE_TRIANGLE,       "Triangle" },
      { MorphLFO::WAVE_SAW_UP,         "Saw Up" },
      { MorphLFO::WAVE_SAW_DOWN,       "Saw Down" },
      { MorphLFO::WAVE_SQUARE,         "Square" },
      { MorphLFO::WAVE_RANDOM_SH,      "Random Sample & Hold" },
      { MorphLFO::WAVE_RANDOM_LINEAR,  "Random Linear" }
    });

  add_property_enum (&m_config.wave_type, P_WAVE_TYPE, "Wave Type", WAVE_SINE, wave_type_enum_info);
  add_property_log (&m_config.frequency, P_FREQUENCY, "Frequency", "%.3f Hz", 1, 0.01, 10);

  auto p_depth = add_property (&m_config.depth, P_DEPTH, "Depth", "-", 1, 0, 1);
  /* FIXME: ideally the storage format should be changed -> store depth as percent */
  p_depth->set_custom_formatter ([](float f) -> string { return string_locale_printf ("%.1f %%", f * 100); });

  add_property (&m_config.center, P_CENTER, "Center", "%.2f", 0, -1, 1);
  add_property (&m_config.start_phase, P_START_PHASE, "Start Phase", "%.1f", 0, -180, 180);

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
  write_properties (out_file);
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
      if (read_property_event (ifile))
        {
          // property has been read, so we ignore the event
        }
      else if (ifile.event() == InFile::INT)
        {
          if (ifile.event_name() == "note")
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
