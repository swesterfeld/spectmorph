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

#include "smmorphsource.hh"
#include "smmorphsourceview.hh"
#include "smmorphplan.hh"

using namespace SpectMorph;

using std::string;

MorphSource::MorphSource (MorphPlan *morph_plan) :
  MorphOperator (morph_plan)
{
}

MorphOperatorView *
MorphSource::create_view (MainWindow *main_window)
{
  return new MorphSourceView (this, main_window);
}

void
MorphSource::set_smset (const string& smset)
{
  m_smset = smset;
  m_morph_plan->emit_plan_changed();
}

string
MorphSource::smset()
{
  return m_smset;
}

const char *
MorphSource::type()
{
  return "SpectMorph::MorphSource";
}

bool
MorphSource::save (OutFile& out_file)
{
  out_file.write_string ("instrument", m_smset);

  return true;
}

bool
MorphSource::load (InFile& ifile)
{
  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::STRING)
        {
          if (ifile.event_name() == "instrument")
            {
              m_smset = ifile.event_data();
            }
          else
            {
              g_printerr ("bad string\n");
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
MorphSource::output_type()
{
  return OUTPUT_AUDIO;
}
