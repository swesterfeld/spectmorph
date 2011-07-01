/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
      else
        {
          g_printerr ("bad event\n");
          return false;
        }
      ifile.next_event();
    }
  return true;
}

void
MorphLFO::post_load()
{
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
