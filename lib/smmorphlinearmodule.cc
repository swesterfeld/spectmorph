// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphlinearmodule.hh"
#include "smmorphlinear.hh"
#include "smmorphplan.hh"
#include "smmorphplanvoice.hh"
#include "smmath.hh"
#include "smleakdebugger.hh"
#include "smlivedecoder.hh"
#include "smmorphutils.hh"
#include "smutils.hh"
#include <glib.h>
#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::min;
using std::max;
using std::sort;

static LeakDebugger leak_debugger ("SpectMorph::MorphLinearModule");

#define DEBUG (0)

MorphLinearModule::MorphLinearModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  my_source.module = this;

  audio.fundamental_freq     = 440;
  audio.mix_freq             = 48000;
  audio.frame_size_ms        = 1;
  audio.frame_step_ms        = 1;
  audio.zeropad              = 4;
  audio.loop_type            = Audio::LOOP_NONE;

  leak_debugger.add (this);
}

MorphLinearModule::~MorphLinearModule()
{
  leak_debugger.del (this);
}

void
MorphLinearModule::set_config (MorphOperator *op)
{
  MorphLinear *linear = dynamic_cast<MorphLinear *> (op);
  MorphOperator *left_op = linear->left_op();
  MorphOperator *right_op = linear->right_op();
  MorphOperator *control_op = linear->control_op();

  if (left_op)
    left_mod = morph_plan_voice->module (left_op);
  else
    left_mod = NULL;

  if (right_op)
    right_mod = morph_plan_voice->module (right_op);
  else
    right_mod = NULL;

  string left_smset = linear->left_smset();
  string right_smset = linear->right_smset();

  have_left_source = (left_smset != "");
  if (have_left_source)
    {
      string smset_dir = linear->morph_plan()->index()->smset_dir();
      string path = smset_dir + "/" + left_smset;

      left_source.set_wav_set (path);
    }

  have_right_source = (right_smset != "");
  if (have_right_source)
    {
      string smset_dir = linear->morph_plan()->index()->smset_dir();
      string path = smset_dir + "/" + right_smset;

      right_source.set_wav_set (path);
    }

  if (control_op)
    control_mod = morph_plan_voice->module (control_op);
  else
    control_mod = NULL;

  clear_dependencies();
  add_dependency (left_mod);
  add_dependency (right_mod);
  add_dependency (control_mod);

  morphing = linear->morphing();
  control_type = linear->control_type();
  db_linear = linear->db_linear();
}

void
MorphLinearModule::MySource::retrigger (int channel, float freq, int midi_velocity, float mix_freq)
{
  if (module->left_mod && module->left_mod->source())
    {
      module->left_mod->source()->retrigger (channel, freq, midi_velocity, mix_freq);
    }

  if (module->right_mod && module->right_mod->source())
    {
      module->right_mod->source()->retrigger (channel, freq, midi_velocity, mix_freq);
    }

  if (module->have_left_source)
    {
      module->left_source.retrigger (channel, freq, midi_velocity, mix_freq);
    }

  if (module->have_right_source)
    {
      module->right_source.retrigger (channel, freq, midi_velocity, mix_freq);
    }
}

Audio*
MorphLinearModule::MySource::audio()
{
  return &module->audio;
}

static void
dump_block (size_t index, const char *what, const AudioBlock& block)
{
  if (DEBUG)
    {
      for (size_t i = 0; i < block.freqs.size(); i++)
        sm_printf ("%zd:%s %.17g %.17g\n", index, what, block.freqs_f (i), block.mags_f (i));
    }
}

static void
dump_line (size_t index, const char *what, double start, double end)
{
  if (DEBUG)
    {
      sm_printf ("%zd:%s %.17g %.17g\n", index, what, start, end);
    }
}

struct MagData
{
  enum {
    BLOCK_LEFT  = 0,
    BLOCK_RIGHT = 1
  }        block;
  size_t   index;
  uint16_t mag;
};

static bool
md_cmp (const MagData& m1, const MagData& m2)
{
  return m1.mag > m2.mag;  // sort with biggest magnitude first
}

void
MorphLinearModule::MySource::interp_mag_one (double interp, uint16_t *left, uint16_t *right)
{
  if (module->db_linear)
    {
      const uint16_t lmag_idb = max<uint16_t> (left ? *left : 0, SM_IDB_CONST_M96);
      const uint16_t rmag_idb = max<uint16_t> (right ? *right : 0, SM_IDB_CONST_M96);

      const uint16_t mag_idb = sm_round_positive ((1 - interp) * lmag_idb + interp * rmag_idb);

      if (left)
        *left = mag_idb;
      if (right)
        *right = mag_idb;
    }
  else
    {
      if (left)
        *left = sm_factor2idb ((1 - interp) * sm_idb2factor (*left));
      if (right)
        *right = sm_factor2idb (interp * sm_idb2factor (*right));
    }
}

AudioBlock *
MorphLinearModule::MySource::audio_block (size_t index)
{
  bool have_left = false, have_right = false;

  double morphing;

  if (module->control_type == MorphLinear::CONTROL_GUI)
    morphing = module->morphing;
  else if (module->control_type == MorphLinear::CONTROL_SIGNAL_1)
    morphing = module->morph_plan_voice->control_input (0);
  else if (module->control_type == MorphLinear::CONTROL_SIGNAL_2)
    morphing = module->morph_plan_voice->control_input (1);
  else if (module->control_type == MorphLinear::CONTROL_OP)
    morphing = module->control_mod->value();
  else
    g_assert_not_reached();

  const double interp = (morphing + 1) / 2; /* examples => 0: only left; 0.5 both equally; 1: only right */
  const double time_ms = index; // 1ms frame step

  Audio *left_audio = nullptr;
  Audio *right_audio = nullptr;
  if (module->left_mod && module->left_mod->source())
    {
      have_left = MorphUtils::get_normalized_block (module->left_mod->source(), time_ms, left_block);
      left_audio = module->left_mod->source()->audio();
    }

  if (module->right_mod && module->right_mod->source())
    {
      have_right = MorphUtils::get_normalized_block (module->right_mod->source(), time_ms, right_block);
      right_audio = module->right_mod->source()->audio();
    }

  if (module->have_left_source)
    {
      have_left = MorphUtils::get_normalized_block (&module->left_source, time_ms, left_block);
      left_audio = module->left_source.audio();
    }

  if (module->have_right_source)
    {
      have_right = MorphUtils::get_normalized_block (&module->right_source, time_ms, right_block);
      right_audio = module->right_source.audio();
    }

  if (have_left && have_right) // true morph: both sources present
    {
      assert (left_audio && right_audio);

      module->audio_block.freqs.clear();
      module->audio_block.mags.clear();
      module->audio_block.phases.clear();

      dump_block (index, "A", left_block);
      dump_block (index, "B", right_block);

      MagData mds[left_block.freqs.size() + right_block.freqs.size()];
      size_t  mds_size = 0;
      for (size_t i = 0; i < left_block.freqs.size(); i++)
        {
          MagData& md = mds[mds_size];

          md.block = MagData::BLOCK_LEFT;
          md.index = i;
          md.mag   = left_block.mags[i];
          mds_size++;
        }
      for (size_t i = 0; i < right_block.freqs.size(); i++)
        {
          MagData& md = mds[mds_size];

          md.block = MagData::BLOCK_RIGHT;
          md.index = i;
          md.mag   = right_block.mags[i];
          mds_size++;
        }
      sort (mds, mds + mds_size, md_cmp);

      size_t    left_freqs_size = left_block.freqs.size();
      size_t    right_freqs_size = right_block.freqs.size();

      MorphUtils::FreqState   left_freqs[left_freqs_size];
      MorphUtils::FreqState   right_freqs[right_freqs_size];

      init_freq_state (left_block.freqs, left_freqs);
      init_freq_state (right_block.freqs, right_freqs);

      for (size_t m = 0; m < mds_size; m++)
        {
          size_t i, j;
          bool match = false;
          if (mds[m].block == MagData::BLOCK_LEFT)
            {
              i = mds[m].index;

              if (!left_freqs[i].used)
                match = MorphUtils::find_match (left_freqs[i].freq_f, right_freqs, right_freqs_size, &j);
            }
          else // (mds[m].block == MagData::BLOCK_RIGHT)
            {
              j = mds[m].index;
              if (!right_freqs[j].used)
                match = MorphUtils::find_match (right_freqs[j].freq_f, left_freqs, left_freqs_size, &i);
            }
          if (match)
            {
              double freq =  (1 - interp) * left_block.freqs[i]  + interp * right_block.freqs[j]; // <- NEEDS better averaging
              double phase = (1 - interp) * left_block.phases[i] + interp * right_block.phases[j];

#if 1
              /* prefer frequency of louder partial */
              const double lfreq = left_block.freqs[i];
              const double rfreq = right_block.freqs[j];

              if (left_block.mags[i] > right_block.mags[j])
                {
                  const double mfact = right_block.mags_f (j) / left_block.mags_f (i);

                  freq = lfreq + mfact * interp * (rfreq - lfreq);
                }
              else
                {
                  const double mfact = left_block.mags_f (i) / right_block.mags_f (j);

                  freq = rfreq + mfact * (1 - interp) * (lfreq - rfreq);
                }
#endif
              double mag;
              if (module->db_linear)
                {
                  // FIXME: this could be faster if we avoided db conversion (see grid morph)

                  double lmag_db = db_from_factor (left_block.mags_f (i), -100);
                  double rmag_db = db_from_factor (right_block.mags_f (j), -100);

                  double mag_db = (1 - interp) * lmag_db + interp * rmag_db;

                  mag = db_to_factor (mag_db);
                }
              else
                {
                  mag = (1 - interp) * left_block.mags_f (i) + interp * right_block.mags_f (j);
                }
              module->audio_block.freqs.push_back (freq);
              module->audio_block.mags.push_back (sm_factor2idb (mag));
              module->audio_block.phases.push_back (phase);
              dump_line (index, "L", left_block.freqs[i], right_block.freqs[j]);
              left_freqs[i].used = 1;
              right_freqs[j].used = 1;
            }
        }
      for (size_t i = 0; i < left_block.freqs.size(); i++)
        {
          if (!left_freqs[i].used)
            {
              module->audio_block.freqs.push_back (left_block.freqs[i]);
              module->audio_block.mags.push_back (left_block.mags[i]);
              module->audio_block.phases.push_back (left_block.phases[i]);

              interp_mag_one (interp, &module->audio_block.mags.back(), NULL);
            }
        }
      for (size_t i = 0; i < right_block.freqs.size(); i++)
        {
          if (!right_freqs[i].used)
            {
              module->audio_block.freqs.push_back (right_block.freqs[i]);
              module->audio_block.mags.push_back (right_block.mags[i]);
              module->audio_block.phases.push_back (right_block.phases[i]);

              interp_mag_one (interp, NULL, &module->audio_block.mags.back());
            }
        }
      assert (left_block.noise.size() == right_block.noise.size());

      module->audio_block.noise.clear();
      for (size_t i = 0; i < left_block.noise.size(); i++)
        module->audio_block.noise.push_back (sm_factor2idb ((1 - interp) * left_block.noise_f (i) + interp * right_block.noise_f (i)));

      module->audio_block.sort_freqs();

      return &module->audio_block;
    }
  else if (have_left) // only left source output present
    {
      module->audio_block = left_block;
      for (size_t i = 0; i < module->audio_block.noise.size(); i++)
        module->audio_block.noise[i] = sm_factor2idb (module->audio_block.noise_f (i) * (1 - interp));
      for (size_t i = 0; i < module->audio_block.freqs.size(); i++)
        interp_mag_one (interp, &module->audio_block.mags[i], NULL);

      return &module->audio_block;
    }
  else if (have_right) // only right source output present
    {
      module->audio_block = right_block;
      for (size_t i = 0; i < module->audio_block.noise.size(); i++)
        module->audio_block.noise[i] = sm_factor2idb (module->audio_block.noise_f (i) * interp);
      for (size_t i = 0; i < module->audio_block.freqs.size(); i++)
        interp_mag_one (interp, NULL, &module->audio_block.mags[i]);

      return &module->audio_block;
    }
  return NULL;
}

LiveDecoderSource *
MorphLinearModule::source()
{
  return &my_source;
}
