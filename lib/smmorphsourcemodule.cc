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

static float
freq_to_note (float freq)
{
  return 69 + 12 * log (freq / 440) / log (2);
}

MorphSourceModule::MorphSourceModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  my_source.wav_set = NULL;
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
  my_source.wav_set = &wav_set;
}

void
MorphSourceModule::MySource::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  Audio *best_audio = NULL;
  float  best_diff  = 1e10;

  if (wav_set)
    {
      float note = freq_to_note (freq);
      for (vector<WavSetWave>::iterator wi = wav_set->waves.begin(); wi != wav_set->waves.end(); wi++)
        {
          Audio *audio = wi->audio;
          if (audio && wi->channel == channel &&
                       wi->velocity_range_min <= midi_velocity &&
                       wi->velocity_range_max >= midi_velocity)
            {
              float audio_note = freq_to_note (audio->fundamental_freq);
              if (fabs (audio_note - note) < best_diff)
                {
                  best_diff = fabs (audio_note - note);
                  best_audio = audio;
                }
            }
        }
    }
  active_audio = best_audio;
}

Audio*
MorphSourceModule::MySource::audio()
{
  return active_audio;
}

AudioBlock *
MorphSourceModule::MySource::audio_block (size_t index)
{
  return &active_audio->contents[index];
}
