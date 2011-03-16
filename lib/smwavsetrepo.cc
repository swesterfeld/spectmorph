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

#include <bse/bseloader.h>
#include "smwavsetrepo.hh"

using namespace SpectMorph;

using std::string;

WavSetRepo*
WavSetRepo::the()
{
  static WavSetRepo *instance = NULL;
  if (!instance)
    instance = new WavSetRepo;

  return instance;
}

WavSet*
WavSetRepo::get (const string& filename)
{
  Birnet::AutoLocker lock (mutex);

  WavSet*& wav_set = wav_set_map[filename];
  if (!wav_set)
    {
      wav_set = new WavSet();
      wav_set->load (filename, AUDIO_SKIP_DEBUG);
    }
  return wav_set;
}
