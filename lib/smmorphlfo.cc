// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphlfo.hh"
#include "smmorphplan.hh"

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphLFO::MorphLFO (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
  EnumInfo wave_type_enum_info (
    {
      { WAVE_SINE,           "Sine" },
      { WAVE_TRIANGLE,       "Triangle" },
      { WAVE_SAW_UP,         "Saw Up" },
      { WAVE_SAW_DOWN,       "Saw Down" },
      { WAVE_SQUARE,         "Square" },
      { WAVE_RANDOM_SH,      "Random Sample & Hold" },
      { WAVE_RANDOM_LINEAR,  "Random Linear" },
      { WAVE_CUSTOM,         "Custom Shape" }
    });
  EnumInfo note_enum_info (
    {
      { NOTE_32_1, "32/1" },
      { NOTE_16_1, "16/1" },
      { NOTE_8_1,  "8/1" },
      { NOTE_4_1,  "4/1" },
      { NOTE_2_1,  "2/1" },
      { NOTE_1_1,  "1/1" },
      { NOTE_1_2,  "1/2" },
      { NOTE_1_4,  "1/4" },
      { NOTE_1_8,  "1/8" },
      { NOTE_1_16, "1/16" },
      { NOTE_1_32, "1/32" },
      { NOTE_1_64, "1/64" }
    });
  EnumInfo note_mode_enum_info (
    {
      { NOTE_MODE_STRAIGHT, "straight" },
      { NOTE_MODE_TRIPLET,  "triplet" },
      { NOTE_MODE_DOTTED,   "dotted" }
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
  add_property_enum (&m_config.note, P_NOTE, "Note", NOTE_1_4, note_enum_info);
  add_property_enum (&m_config.note_mode, P_NOTE_MODE, "Note Mode", NOTE_MODE_STRAIGHT, note_mode_enum_info);

  m_config.curve.points.emplace_back (Curve::Point {0, 0});
  m_config.curve.points.emplace_back (Curve::Point {0, 1, -0.6});
  m_config.curve.points.emplace_back (Curve::Point {0.5, 0});
  m_config.curve.points.emplace_back (Curve::Point {0.5, 0.75, -0.6});
  m_config.curve.points.emplace_back (Curve::Point {0.75, 0});
  m_config.curve.points.emplace_back (Curve::Point {0.75, 0.5, -0.6});
  m_config.curve.points.emplace_back (Curve::Point {1, 0});
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
  m_config.curve.save ("curve", out_file);

  return true;
}

bool
MorphLFO::load (InFile& ifile)
{
  bool read_ok = true;
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (read_property_event (ifile) || m_config.curve.load ("curve", ifile))
        {
          // property has been read, so we ignore the event
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
              report_bad_event (read_ok, ifile);
            }
        }
      else
        {
          report_bad_event (read_ok, ifile);
        }
      ifile.next_event();
    }
  return read_ok;
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

const Curve&
MorphLFO::curve() const
{
  return m_config.curve;
}

void
MorphLFO::set_curve (const Curve& curve)
{
  m_config.curve = curve;
  m_morph_plan->emit_plan_changed();
}

bool
MorphLFO::custom_shape() const
{
  return m_config.wave_type == WAVE_CUSTOM;
}

MorphOperatorConfig *
MorphLFO::clone_config()
{
  return new Config (m_config);
}
