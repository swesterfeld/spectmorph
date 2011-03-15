/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
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

#ifndef SPECTMORPH_WAVSET_REPO_HH
#define SPECTMORPH_WAVSET_REPO_HH

#include "smwavset.hh"

#include <bse/bsecxxplugin.hh>

namespace SpectMorph
{

class WavSetRepo {
  Birnet::Mutex mutex;
  std::map<std::string, WavSet *> wav_set_map;
public:
  WavSet *get (const std::string& filename);

  static WavSetRepo *the(); // Singleton
};

}

#endif
