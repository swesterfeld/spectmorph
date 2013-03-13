// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridmodule.hh"
#include "smmorphgrid.hh"
#include "smmorphplanvoice.hh"
#include "smleakdebugger.hh"

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::MorphGridModule");

MorphGridModule::MorphGridModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice, 4)
{
  leak_debugger.add (this);
}

MorphGridModule::~MorphGridModule()
{
  leak_debugger.del (this);
}

void
MorphGridModule::set_config (MorphOperator *op)
{
  MorphGrid *grid = dynamic_cast<MorphGrid *> (op);

  input_mod.resize (grid->width());

  for (int x = 0; x < grid->width(); x++)
    {
      input_mod[x].resize (grid->height());
      for (int y = 0; y < grid->height(); y++)
        {
          MorphOperator *input_op = grid->input_op (x, y);

          if (input_op)
            {
              input_mod[x][y] = morph_plan_voice->module (input_op);
            }
          else
            {
              input_mod[x][y] = NULL;
            }
        }
    }
}

void
MorphGridModule::MySource::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
}

Audio*
MorphGridModule::MySource::audio()
{
}

AudioBlock *
MorphGridModule::MySource::audio_block (size_t index)
{
}
