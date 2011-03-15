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

#include "smmorphsourcemodule.hh"
#include "smmorphsource.hh"
#include "smmorphplan.hh"
#include <glib.h>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphSourceModule::MorphSourceModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
}

LiveDecoderSource *
MorphSourceModule::source()
{
  return &my_source;
}

void
MorphSourceModule::set_config (MorphOperator *op)
{
  MorphSource *source = dynamic_cast<MorphSource *> (op);
  string smset = source->smset();
  string smset_dir = source->morph_plan()->index()->smset_dir();
  string path = smset_dir + "/" + smset;
  g_printerr ("loading %s...\n", path.c_str());
  wav_set.load (path);
}


