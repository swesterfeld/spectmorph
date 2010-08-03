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

#include "smgenericin.hh"
#include "smmmapin.hh"
#include "smstdioin.hh"

using SpectMorph::GenericIn;
using SpectMorph::MMapIn;
using SpectMorph::StdioIn;
using std::string;

GenericIn*
GenericIn::open (const std::string& filename)
{
  GenericIn *file = MMapIn::open (filename);
  if (!file)
    file = StdioIn::open (filename);
  return file;
}


