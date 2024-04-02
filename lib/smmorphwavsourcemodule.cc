// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphwavsourcemodule.hh"
#include "smmorphwavsource.hh"
#include "smmorphplan.hh"
#include "smleakdebugger.hh"
#include "sminstrument.hh"
#include "smwavsetbuilder.hh"
#include "smmorphplanvoice.hh"
#include <glib.h>
#include <thread>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;
using std::min;

static LeakDebugger leak_debugger ("SpectMorph::MorphWavSourceModule");

void
VoiceSource::set_ratio (double ratio)
{
  m_ratio = ratio;
}

void
VoiceSource::retrigger()
{
  gen_detune_factors (detune_factors);
  gen_detune_factors (next_detune_factors);
}

void
VoiceSource::gen_detune_factors (vector<float>& factors)
{
  const double max_fuzzy_resynth = 50; // cent
  factors.resize (400);
  for (int i = 1; i < 400; i++)
    {
      double fuzzy_high = pow (2, fuzzy_resynth / 1200);
      double fuzzy_low = 1 / fuzzy_high;
      double fuzzy_high_bound = 1 + (pow (2, max_fuzzy_resynth / 1200) - 1) / i;
      double fuzzy_low_bound = 1 / fuzzy_high_bound;

      factors[i] = detune_random.random_double_range (max (fuzzy_low, fuzzy_low_bound), min (fuzzy_high, fuzzy_high_bound));
    }
}

void
VoiceSource::advance (double time_ms)
{
  const double fuzzy_resynth_freq = detune_random.random_double_range (6, 10); // Hz
  fuzzy_frac += 0.001 * time_ms * fuzzy_resynth_freq;
}

void
VoiceSource::process_block (const AudioBlock& in_block, RTAudioBlock& out_block)
{
  /* compute energy before formant correction */
  double e1 = 0;
  for (size_t i = 0; i < in_block.mags.size(); i++)
    {
      double mag = in_block.mags_f (i);
      e1 += mag * mag;
    }
  if (mode == MorphWavSource::FORMANT_ENVELOPE)
    {
      const double e_tune_factor = 1 / in_block.env_f0;

      out_block.assign (in_block);
      for (size_t i = 0; i < out_block.freqs.size(); i++)
        {
          auto emag = [&] (int i) {
            if (i > 0 && i < int (in_block.env.size()))
              return in_block.env_f (i);
            return 0.0;
          };
          auto emag_inter  = [&] (double p) {
            int ip = p;
            double frac = p - ip;
            return emag (ip) * (1 - frac) + emag (ip + 1) * frac;
          };
          double freq = out_block.freqs_f (i) * e_tune_factor;
          double old_env_mag = emag_inter (freq);
          double new_env_mag = emag_inter (freq * m_ratio);

          if (freq < 0.5)
            out_block.mags[i] = sm_factor2idb (in_block.mags_f (i) * 0.0001);
          else
            out_block.mags[i] = sm_factor2idb (in_block.mags_f (i) / old_env_mag * new_env_mag);
        }
    }
  else if (mode == MorphWavSource::FORMANT_RESYNTH)
    {
      out_block.mags.set_capacity (400);
      out_block.freqs.set_capacity (400);
      out_block.noise.assign (in_block.noise);
      double ff = in_block.env_f0;
      if (fuzzy_frac > 1)
        {
          detune_factors.swap (next_detune_factors);
          gen_detune_factors (next_detune_factors);
          fuzzy_frac -= int (fuzzy_frac);
        }
      for (int i = 1; i < 400; i++)
        {
          auto emag = [&] (int i) {
            if (i > 0 && i < int (in_block.env.size()))
              return in_block.env_f (i);
            return 0.0;
          };
          auto emag_inter  = [&] (double p) {
            int ip = p;
            double frac = p - ip;
            return emag (ip) * (1 - frac) + emag (ip + 1) * frac;
          };
          out_block.freqs.push_back (sm_freq2ifreq (i * ff * (detune_factors[i] * (1 - fuzzy_frac) + next_detune_factors[i] * fuzzy_frac)));
          out_block.mags.push_back (sm_factor2idb (emag_inter (i * m_ratio)));
          if (i * m_ratio > max_partials)
            break;
        }
    }
  /* compute energy after formant correction */
  double e2 = 0;
  for (size_t i = 0; i < out_block.mags.size(); i++)
    {
      double mag = out_block.mags_f (i);
      e2 += mag * mag;
    }
  /* normalize block energy */
  // TODO: are the thresholds good enough?
  const double threshold = 1e-9;
  double norm = (e2 > threshold && e2 > threshold) ? sqrt (e1 / e2) : 1;
  for (size_t i = 0; i < out_block.mags.size(); i++)
    out_block.mags[i] = sm_factor2idb (out_block.mags_f (i) * norm);
}

void
MorphWavSourceModule::InstrumentSource::retrigger (int channel, float freq, int midi_velocity)
{
  Audio  *best_audio = nullptr;
  float   best_diff  = 1e10;

  WavSet *wav_set = project->get_wav_set (object_id);
  if (wav_set)
    {
      float note = sm_freq_to_note (freq);
      for (vector<WavSetWave>::iterator wi = wav_set->waves.begin(); wi != wav_set->waves.end(); wi++)
        {
          Audio *audio = wi->audio;
          if (audio && wi->channel == channel &&
                       wi->velocity_range_min <= midi_velocity &&
                       wi->velocity_range_max >= midi_velocity)
            {
              float audio_note = sm_freq_to_note (audio->fundamental_freq);
              if (fabs (audio_note - note) < best_diff)
                {
                  best_diff = fabs (audio_note - note);
                  best_audio = audio;
                }
            }
        }
    }
  active_audio = best_audio;
  if (best_audio)
    {
      voice_source.set_ratio (freq / best_audio->fundamental_freq);
      voice_source.set_max_partials (24000 / best_audio->fundamental_freq);
      voice_source.retrigger();
    }
}

Audio*
MorphWavSourceModule::InstrumentSource::audio()
{
  WavSet *wav_set = project->get_wav_set (object_id);
  if (!wav_set)
    active_audio = nullptr;

  return active_audio;
}

bool
MorphWavSourceModule::InstrumentSource::rt_audio_block (size_t index, RTAudioBlock& out_block)
{
  WavSet *wav_set = project->get_wav_set (object_id);
  if (!wav_set)
    active_audio = nullptr;

  if (active_audio && module->cfg->play_mode == MorphWavSource::PLAY_MODE_CUSTOM_POSITION)
    {
      const double position = module->apply_modulation (module->cfg->position_mod) * 0.01;

      int start, end;
      if (active_audio->loop_type == Audio::LOOP_NONE)
        {
          // play everything
          start = 0;
          end = active_audio->contents.size() - 1;
        }
      else
        {
          // play loop
          start = active_audio->loop_start;
          end = active_audio->loop_end;
        }
      index = sm_bound (start, sm_round_positive ((1 - position) * start + position * end), end);
    }
  if (active_audio && index < active_audio->contents.size())
    {
      if (module->cfg->formant_correct == MorphWavSource::FORMANT_REPITCH)
        out_block.assign (active_audio->contents[index]);
      else
        {
          voice_source.advance (module->time_info().time_ms - last_time_ms);
          last_time_ms = module->time_info().time_ms;
          voice_source.process_block (active_audio->contents[index], out_block);
        }
      return true;
    }
  else
    {
      return false;
    }
}

void
MorphWavSourceModule::InstrumentSource::update_voice_source (const MorphWavSource::Config *config)
{
  voice_source.set_mode (config->formant_correct);
  voice_source.set_fuzzy_resynth (config->fuzzy_resynth);
}

void
MorphWavSourceModule::InstrumentSource::update_project_and_object_id (Project *new_project, int new_object_id)
{
  project = new_project;
  object_id = new_object_id;

  WavSet *wav_set = project->get_wav_set (object_id);
  if (!wav_set)
    active_audio = nullptr;
}

MorphWavSourceModule::MorphWavSourceModule (MorphPlanVoice *voice) :
  MorphOperatorModule (voice)
{
  my_source.module = this;

  leak_debugger.add (this);
}

MorphWavSourceModule::~MorphWavSourceModule()
{
  leak_debugger.del (this);
}

LiveDecoderSource *
MorphWavSourceModule::source()
{
  return &my_source;
}

void
MorphWavSourceModule::set_config (const MorphOperatorConfig *op_cfg)
{
  cfg = dynamic_cast<const MorphWavSource::Config *> (op_cfg);

  my_source.update_project_and_object_id (cfg->project, cfg->object_id);
  my_source.update_voice_source (cfg);
}
