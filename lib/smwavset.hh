/* 
 * Copyright (C) 2010 Stefan Westerfeld
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

#ifndef SPECTMORPH_WAVSET_HH
#define SPECTMORPH_WAVSET_HH

#include <vector>
#include <string>
#include <bse/bsecxxplugin.hh>

#include "smaudio.hh"

namespace SpectMorph
{

class WavSetWave
{
public:
  int         midi_note;
  int         channel;
  int         velocity_range_min;
  int         velocity_range_max;
  std::string path;
  Audio      *audio;

  WavSetWave();
  ~WavSetWave();
};

class WavSet
{
public:
  ~WavSet();

  std::vector<WavSetWave>  waves;

  BseErrorType load (const std::string& filename);
  BseErrorType save (const std::string& filename, bool embed_models = false);
};

}

#endif
