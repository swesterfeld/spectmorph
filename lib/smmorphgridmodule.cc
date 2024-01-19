// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphgridmodule.hh"
#include "smmorphgrid.hh"
#include "smmorphplanvoice.hh"
#include "smleakdebugger.hh"
#include "smmath.hh"
#include "smlivedecoder.hh"
#include "smmorphutils.hh"

#include <assert.h>

using namespace SpectMorph;

using std::min;
using std::max;
using std::vector;
using std::string;
using std::sort;

static LeakDebugger leak_debugger ("SpectMorph::MorphGridModule");

MorphGridModule::MorphGridModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  leak_debugger.add (this);

  my_source.module = this;

  audio.fundamental_freq     = 440;
  audio.mix_freq             = 48000;
  audio.frame_size_ms        = 1;
  audio.frame_step_ms        = 1;
  audio.zeropad              = 4;
  audio.loop_type            = Audio::LOOP_NONE;
}

MorphGridModule::~MorphGridModule()
{
  leak_debugger.del (this);
}

void
MorphGridModule::set_config (const MorphOperatorConfig *op_cfg)
{
  cfg = dynamic_cast<const MorphGrid::Config *> (op_cfg);
  g_return_if_fail (cfg != NULL);

  for (int x = 0; x < cfg->width; x++)
    {
      for (int y = 0; y < cfg->height; y++)
        {
          const MorphGridNode& node = cfg->input_node[x][y];

          input_nodes (x, y).mod = morph_plan_voice->module (node.op);

          if (node.wav_set)
            {
              input_nodes (x, y).source.set_wav_set (node.wav_set);
              input_nodes (x, y).has_source = true;
            }
          else
            {
              input_nodes (x, y).has_source = false;
            }

          input_nodes (x, y).delta_db = node.delta_db;
        }
    }
}

void
MorphGridModule::MySource::retrigger (int channel, float freq, int midi_velocity)
{
  for (int x = 0; x < module->cfg->width; x++)
    {
      for (int y = 0; y < module->cfg->height; y++)
        {
          InputNode& node = module->input_nodes (x, y);

          if (node.mod && node.mod->source())
            {
              node.mod->source()->retrigger (channel, freq, midi_velocity);
            }
          if (node.has_source)
            {
              node.source.retrigger (channel, freq, midi_velocity);
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
get_normalized_block (MorphGridModule::InputNode& input_node, size_t index, RTAudioBlock& out_audio_block)
{
  LiveDecoderSource *source = NULL;

  if (input_node.mod)
    {
      source = input_node.mod->source();
    }
  else if (input_node.has_source)
    {
      source = &input_node.source;
    }
  const double time_ms = index; // 1ms frame step

  return MorphUtils::get_normalized_block (source, time_ms, out_audio_block);
}

namespace
{

struct LocalMorphParams
{
  int     start;
  int     end;
  double  morphing;
};

static LocalMorphParams
global_to_local_params (double global_morphing, int node_count)
{
  LocalMorphParams result;

  /* interp: range for node_count=3: 0 ... 2.0 */
  const double interp = (global_morphing + 1) / 2 * (node_count - 1);

  // find the two adjecant nodes (double -> integer position)
  result.start = sm_bound<int> (0, interp, (node_count - 1));
  result.end   = sm_bound<int> (0, result.start + 1, (node_count - 1));

  const double interp_frac = sm_bound (0.0, interp - result.start, 1.0); /* position between adjecant nodes */
  result.morphing = interp_frac * 2 - 1; /* normalize fractional part to range -1.0 ... 1.0 */
  return result;
}

static double
morph_delta_db (double left_db, double right_db, double morphing)
{
  const double interp = (morphing + 1) / 2; /* examples => 0: only left; 0.5 both equally; 1: only right */
  return left_db * (1 - interp) + right_db * interp;
}

static void
apply_delta_db (RTAudioBlock& block, double delta_db)
{
  const double factor = db_to_factor (delta_db);
  const int    ddb    = sm_factor2delta_idb (factor);

  // apply delta db volume to partials & noise
  for (size_t i = 0; i < block.mags.size(); i++)
    block.mags[i] = sm_bound<int> (0, block.mags[i] + ddb, 65535);

  for (size_t i = 0; i < block.noise.size(); i++)
    block.noise[i] = sm_bound<int> (0, block.noise[i] + ddb, 65535);
}

}

bool
MorphGridModule::MySource::rt_audio_block (size_t index, RTAudioBlock& out_block)
{
  const double x_morphing = module->apply_modulation (module->cfg->x_morphing_mod);
  const double y_morphing = module->apply_modulation (module->cfg->y_morphing_mod);
  const MorphUtils::MorphMode morph_mode = MorphUtils::MorphMode::DB_LINEAR;

  const LocalMorphParams x_morph_params = global_to_local_params (x_morphing, module->cfg->width);
  const LocalMorphParams y_morph_params = global_to_local_params (y_morphing, module->cfg->height);

  if (module->cfg->height == 1)
    {
      RTAudioBlock audio_block_a (module->rt_memory_area());
      RTAudioBlock audio_block_b (module->rt_memory_area());

      /*
       *  A ---- B
       */

      InputNode& node_a = module->input_nodes (x_morph_params.start, 0);
      InputNode& node_b = module->input_nodes (x_morph_params.end,   0);

      bool have_a = get_normalized_block (node_a, index, audio_block_a);
      bool have_b = get_normalized_block (node_b, index, audio_block_b);

      bool have_ab = MorphUtils::morph (out_block, have_a, audio_block_a, have_b, audio_block_b, x_morph_params.morphing, morph_mode);
      if (have_ab)
        {
          double delta_db = morph_delta_db (node_a.delta_db, node_b.delta_db, x_morph_params.morphing);

          apply_delta_db (out_block, delta_db);
          return true;
        }
      else
        {
          return false;
        }
    }
  else if (module->cfg->width == 1)
    {
      RTAudioBlock audio_block_a (module->rt_memory_area());
      RTAudioBlock audio_block_b (module->rt_memory_area());

      /*
       *  A
       *  |
       *  |
       *  B
       */

      InputNode& node_a = module->input_nodes (0, y_morph_params.start);
      InputNode& node_b = module->input_nodes (0, y_morph_params.end);

      bool have_a = get_normalized_block (node_a, index, audio_block_a);
      bool have_b = get_normalized_block (node_b, index, audio_block_b);

      bool have_ab = MorphUtils::morph (out_block, have_a, audio_block_a, have_b, audio_block_b, y_morph_params.morphing, morph_mode);
      if (have_ab)
        {
          double delta_db = morph_delta_db (node_a.delta_db, node_b.delta_db, y_morph_params.morphing);

          apply_delta_db (out_block, delta_db);
          return true;
        }
      else
        {
          return false;
        }
    }
  else
    {
      RTAudioBlock audio_block_a (module->rt_memory_area());
      RTAudioBlock audio_block_b (module->rt_memory_area());
      RTAudioBlock audio_block_c (module->rt_memory_area());
      RTAudioBlock audio_block_d (module->rt_memory_area());
      RTAudioBlock audio_block_ab (module->rt_memory_area());
      RTAudioBlock audio_block_cd (module->rt_memory_area());

      /*
       *  A ---- B
       *  |      |
       *  |      |
       *  C ---- D
       */
      InputNode& node_a = module->input_nodes (x_morph_params.start, y_morph_params.start);
      InputNode& node_b = module->input_nodes (x_morph_params.end,   y_morph_params.start);
      InputNode& node_c = module->input_nodes (x_morph_params.start, y_morph_params.end);
      InputNode& node_d = module->input_nodes (x_morph_params.end,   y_morph_params.end);

      bool have_a = get_normalized_block (node_a, index, audio_block_a);
      bool have_b = get_normalized_block (node_b, index, audio_block_b);
      bool have_c = get_normalized_block (node_c, index, audio_block_c);
      bool have_d = get_normalized_block (node_d, index, audio_block_d);

      bool have_ab = MorphUtils::morph (audio_block_ab, have_a, audio_block_a, have_b, audio_block_b, x_morph_params.morphing, morph_mode);
      bool have_cd = MorphUtils::morph (audio_block_cd, have_c, audio_block_c, have_d, audio_block_d, x_morph_params.morphing, morph_mode);
      bool have_abcd = MorphUtils::morph (out_block, have_ab, audio_block_ab, have_cd, audio_block_cd, y_morph_params.morphing, morph_mode);

      if (have_abcd)
        {
          double delta_db_ab = morph_delta_db (node_a.delta_db, node_b.delta_db, x_morph_params.morphing);
          double delta_db_cd = morph_delta_db (node_c.delta_db, node_d.delta_db, x_morph_params.morphing);
          double delta_db_abcd = morph_delta_db (delta_db_ab, delta_db_cd, y_morph_params.morphing);

          apply_delta_db (out_block, delta_db_abcd);
          return true;
        }
      else
        {
          return false;
        }
    }
}

LiveDecoderSource *
MorphGridModule::source()
{
  return &my_source;
}
