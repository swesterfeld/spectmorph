// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridmodule.hh"
#include "smmorphgrid.hh"
#include "smmorphplanvoice.hh"
#include "smleakdebugger.hh"
#include "smmath.hh"
#include "smlivedecoder.hh"

#include <assert.h>

using namespace SpectMorph;

using std::min;

static LeakDebugger leak_debugger ("SpectMorph::MorphGridModule");

MorphGridModule::MorphGridModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice, 4)
{
  leak_debugger.add (this);

  width = 0;
  height = 0;

  my_source.module = this;

  audio.fundamental_freq     = 440;
  audio.mix_freq             = 48000;
  audio.frame_size_ms        = 1;
  audio.frame_step_ms        = 1;
  audio.attack_start_ms      = 0;
  audio.attack_end_ms        = 0;
  audio.zeropad              = 4;
  audio.loop_type            = Audio::LOOP_NONE;
  audio.zero_values_at_start = 0;
  audio.sample_count         = 2 << 31;
  audio.start_ms             = 0;
}

MorphGridModule::~MorphGridModule()
{
  leak_debugger.del (this);
}

void
MorphGridModule::set_config (MorphOperator *op)
{
  MorphGrid *grid = dynamic_cast<MorphGrid *> (op);

  width = grid->width();
  height = grid->height();

  input_mod.resize (width);

  for (size_t x = 0; x < width; x++)
    {
      input_mod[x].resize (height);
      for (size_t y = 0; y < height; y++)
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

  x_morphing = grid->x_morphing();
  y_morphing = grid->y_morphing();

  update_dependency (0, input_mod[0][0]);
  update_dependency (1, input_mod[1][0]);
  update_dependency (2, input_mod[0][1]);
  update_dependency (3, input_mod[1][1]);
}

void
MorphGridModule::MySource::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  for (size_t x = 0; x < module->width; x++)
    {
      for (size_t y = 0; y < module->height; y++)
        {
          MorphOperatorModule *input_mod = module->input_mod[x][y];

          if (input_mod && input_mod->source())
            {
              input_mod->source()->retrigger (channel, freq, midi_velocity, mix_freq);
            }
        }
    }
}

Audio*
MorphGridModule::MySource::audio()
{
  return &module->audio;
}

static bool
get_normalized_block (LiveDecoderSource *source, size_t index, AudioBlock& out_audio_block, int delay_blocks)
{
  g_return_val_if_fail (delay_blocks >= 0, false);
  if (index < delay_blocks)
    {
      out_audio_block.noise.resize (32);
      return true;  // morph with empty block to ensure correct time alignment
    }
  else
    {
      // shift audio for time alignment
      assert (index >= delay_blocks);
      index -= delay_blocks;
    }

  Audio *audio = source->audio();
  if (!audio)
    return false;

  if (audio->loop_type == Audio::LOOP_TIME_FORWARD)
    {
      size_t loop_start_index = sm_round_positive (audio->loop_start * 1000.0 / audio->mix_freq);
      size_t loop_end_index   = sm_round_positive (audio->loop_end   * 1000.0 / audio->mix_freq);

      if (loop_start_index >= loop_end_index)
        {
          /* loop_start_index usually should be less than loop_end_index, this is just
           * to handle corner cases and pathological cases
           */
          index = min (index, loop_start_index);
        }
      else
        {
          while (index >= loop_end_index)
            {
              index -= (loop_end_index - loop_start_index);
            }
        }
    }

  double time_ms = index; // 1ms frame step
  int source_index = sm_round_positive (time_ms / audio->frame_step_ms);

  if (audio->loop_type == Audio::LOOP_FRAME_FORWARD || audio->loop_type == Audio::LOOP_FRAME_PING_PONG)
    {
      source_index = LiveDecoder::compute_loop_frame_index (source_index, audio);
    }

  AudioBlock *block_ptr = source->audio_block (source_index);

  if (!block_ptr)
    return false;

  out_audio_block.noise  = block_ptr->noise;
  out_audio_block.mags   = block_ptr->mags;
  out_audio_block.phases = block_ptr->phases;  // usually not used
  out_audio_block.freqs.resize (block_ptr->freqs.size());

  for (size_t i = 0; i < block_ptr->freqs.size(); i++)
    out_audio_block.freqs[i] = block_ptr->freqs[i] * 440 / audio->fundamental_freq;

  out_audio_block.lpc_lsf_p = block_ptr->lpc_lsf_p;
  out_audio_block.lpc_lsf_q = block_ptr->lpc_lsf_q;

  return true;
}

bool
morph (AudioBlock& out,
       bool have_left, const AudioBlock& left_block,
       bool have_right, const AudioBlock& right_block,
       double morphing)
{
  const double interp = (morphing + 1) / 2; /* examples => 0: only left; 0.5 both equally; 1: only right */

  if (interp < 0.5)
    {
      out = left_block;
      return have_left;
    }
  else
    {
      out = right_block;
      return have_right;
    }
}

AudioBlock *
MorphGridModule::MySource::audio_block (size_t index)
{
  assert (module->width == 2);
  assert (module->height == 2);

  AudioBlock audio_block_a, audio_block_b, audio_block_c, audio_block_d;

  /*
   *  A ---- B
   *  |      |
   *  |      |
   *  C ---- D
   */
  bool have_a = get_normalized_block (module->input_mod[0][0]->source(), index, audio_block_a, 0);
  bool have_b = get_normalized_block (module->input_mod[1][0]->source(), index, audio_block_b, 0);
  bool have_c = get_normalized_block (module->input_mod[0][1]->source(), index, audio_block_c, 0);
  bool have_d = get_normalized_block (module->input_mod[1][1]->source(), index, audio_block_d, 0);

  const double x_morphing = module->x_morphing;
  const double y_morphing = module->y_morphing;

  AudioBlock audio_block_ab, audio_block_cd;
  bool have_ab = morph (audio_block_ab, have_a, audio_block_a, have_b, audio_block_b, x_morphing);
  bool have_cd = morph (audio_block_cd, have_c, audio_block_c, have_d, audio_block_d, x_morphing);
  bool have_abcd = morph (module->audio_block, have_ab, audio_block_ab, have_cd, audio_block_cd, y_morphing);

  return have_abcd ? &module->audio_block : NULL;
}

LiveDecoderSource *
MorphGridModule::source()
{
  return &my_source;
}
