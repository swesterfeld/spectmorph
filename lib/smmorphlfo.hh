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

#ifndef SPECTMORPH_MORPH_LFO_HH
#define SPECTMORPH_MORPH_LFO_HH

#include "smmorphoperator.hh"

#include <string>

namespace SpectMorph
{

class MorphLFO : public MorphOperator
{
public:
  enum WaveType {
    WAVE_SINE        = 1,
    WAVE_TRIANGLE    = 2,
  };
protected:
  WaveType       m_wave_type;

public:
  MorphLFO (MorphPlan *morph_plan);
  ~MorphLFO();

  // inherited from MorphOperator
  const char        *type();
  bool               save (OutFile& out_file);
  bool               load (InFile&  in_file);
  void               post_load();
  OutputType         output_type();

  WaveType wave_type();
  void set_wave_type (WaveType new_wave_type);
};

}

#endif
