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
#include "smmorphplan.hh"
#include "smmorphoutput.hh"
#include "smmorphoperatormodule.hh"
#include "smmorphoutputmodule.hh"
#include "smmorphsource.hh"
#include "smmain.hh"

#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::string;

namespace SpectMorph
{

class MorphLinearModule : public MorphOperatorModule
{
public:
  MorphLinearModule (MorphPlanVoice *voice);

  void set_config (MorphOperator *op);
};

MorphLinearModule::MorphLinearModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
}

void
MorphLinearModule::set_config (MorphOperator *op)
{
}

class MorphSourceModule : public MorphOperatorModule
{
protected:
  WavSet wav_set;

  struct MySource : public LiveDecoderSource
  {
    void
    retrigger (int channel, float freq, int midi_velocity, float mix_freq)
    {
      g_printerr ("retrigger %d, %f, %f\n", channel, freq, mix_freq);
    }
    Audio* audio() { return NULL; }
    AudioBlock *audio_block (size_t index) { return NULL; }
  } my_source;
public:
  MorphSourceModule (MorphPlanVoice *voice);

  void set_config (MorphOperator *op);
  LiveDecoderSource *source();
};

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

MorphOutputModule::MorphOutputModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
}

void
MorphOutputModule::set_config (MorphOperator *op)
{
  MorphOutput *out_op = dynamic_cast <MorphOutput *> (op);
  g_return_if_fail (out_op != NULL);

  out_ops.clear();
  out_decoders.clear(); // FIXME: LEAK ?
  for (size_t ch = 0; ch < 4; ch++)
    {
      MorphOperatorModule *mod = NULL;
      LiveDecoder *dec = NULL;

      MorphOperator *op = out_op->channel_op (ch);
      if (op)
        {
          mod = morph_plan_voice->module (op);
          dec = new LiveDecoder (mod->source());
        }

      out_ops.push_back (mod);
      out_decoders.push_back (dec);
    }
}

void
MorphOutputModule::process (size_t n_values, float *values)
{
  g_printerr ("process called; out_ops[0] = %p\n", out_ops[0]);
  out_decoders[0]->retrigger (0, 440, 127, 48000);
  out_decoders[0]->process (n_values, 0, 0, values);
}

}

MorphOperatorModule*
MorphOperatorModule::create (MorphOperator *op, MorphPlanVoice *voice)
{
  string type = op->type();

  if (type == "SpectMorph::MorphLinear") return new MorphLinearModule (voice);
  if (type == "SpectMorph::MorphSource") return new MorphSourceModule (voice);
  if (type == "SpectMorph::MorphOutput") return new MorphOutputModule (voice);

  return NULL;
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);
  if (argc != 2)
    {
      printf ("usage: %s <plan>\n", argv[0]);
      exit (1);
    }

  MorphPlan plan;
  GenericIn *in = StdioIn::open (argv[1]);
  if (!in)
    {
      g_printerr ("Error opening '%s'.\n", argv[1]);
      exit (1);
    }
  plan.load (in);
  fprintf (stderr, "SUCCESS: plan loaded, %zd operators found.\n", plan.operators().size());

  MorphPlanVoice voice (&plan);
  assert (voice.output());

  vector<float> samples (44100);
  voice.output()->process (samples.size(), &samples[0]);
  for (size_t i = 0; i < samples.size(); i++)
    {
      printf ("%.17g\n", samples[i]);
    }
}
