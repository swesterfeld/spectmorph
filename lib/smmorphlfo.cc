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
  m_wave_type = WAVE_SINE;
  m_frequency = 1;
  m_depth = 1;
  m_center = 0;
  m_start_phase = 0;
  m_sync_voices = false;

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

bool
MorphLFO::save (OutFile& out_file)
{
  out_file.write_int ("wave_type", m_wave_type);
  out_file.write_float ("frequency", m_frequency);
  out_file.write_float ("depth", m_depth);
  out_file.write_float ("center", m_center);
  out_file.write_float ("start_phase", m_start_phase);
  out_file.write_bool ("sync_voices", m_sync_voices);

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
              m_wave_type = static_cast<WaveType> (ifile.event_int());
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
              m_frequency = ifile.event_float();
            }
          else if (ifile.event_name() == "depth")
            {
              m_depth = ifile.event_float();
            }
          else if (ifile.event_name() == "center")
            {
              m_center = ifile.event_float();
            }
          else if (ifile.event_name() == "start_phase")
            {
              m_start_phase = ifile.event_float();
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
              m_sync_voices = ifile.event_bool();
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
  return m_wave_type;
}

void
MorphLFO::set_wave_type (WaveType new_wave_type)
{
  m_wave_type = new_wave_type;

  m_morph_plan->emit_plan_changed();
}

float
MorphLFO::frequency() const
{
  return m_frequency;
}

void
MorphLFO::set_frequency (float frequency)
{
  m_frequency = frequency;

  m_morph_plan->emit_plan_changed();
}

float
MorphLFO::depth() const
{
  return m_depth;
}

void
MorphLFO::set_depth (float depth)
{
  m_depth = depth;

  m_morph_plan->emit_plan_changed();
}

float
MorphLFO::center() const
{
  return m_center;
}

void
MorphLFO::set_center (float center)
{
  m_center = center;

  m_morph_plan->emit_plan_changed();
}

float
MorphLFO::start_phase() const
{
  return m_start_phase;
}

void
MorphLFO::set_start_phase (float start_phase)
{
  m_start_phase = start_phase;

  m_morph_plan->emit_plan_changed();
}

bool
MorphLFO::sync_voices() const
{
  return m_sync_voices;
}

void
MorphLFO::set_sync_voices (float sync_voices)
{
  m_sync_voices = sync_voices;

  m_morph_plan->emit_plan_changed();
}
