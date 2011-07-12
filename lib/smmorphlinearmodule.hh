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

#ifndef SPECTMORPH_MORPH_LINEAR_MODULE_HH
#define SPECTMORPH_MORPH_LINEAR_MODULE_HH

#include "smmorphplan.hh"
#include "smmorphoutput.hh"
#include "smmorphoperatormodule.hh"
#include "smmorphlinear.hh"

namespace SpectMorph
{

class MorphLinearModule : public MorphOperatorModule
{
  MorphOperatorModule *left_mod;
  MorphOperatorModule *right_mod;
  MorphOperatorModule *control_mod;
  float                morphing;
  bool                 db_linear;
  bool                 use_lpc;

  MorphLinear::ControlType control_type;

  Audio                audio;
  AudioBlock           audio_block;

  int                  left_delay_blocks;
  int                  right_delay_blocks;

  struct MySource : public LiveDecoderSource
  {
    MorphLinearModule *module;

    void interp_mag_one (double interp, float *left, float *right);
    void retrigger (int channel, float freq, int midi_velocity, float mix_freq);
    Audio* audio();
    AudioBlock *audio_block (size_t index);
  } my_source;

public:
  MorphLinearModule (MorphPlanVoice *voice);
  ~MorphLinearModule();

  void set_config (MorphOperator *op);
  LiveDecoderSource *source();
};

}

#endif
