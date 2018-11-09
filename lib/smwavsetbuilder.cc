// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavsetbuilder.hh"
#include "sminstencoder.hh"
#include "smbinbuffer.hh"
#include "sminstenccache.hh"
#include "smaudiotool.hh"

#include <mutex>

using namespace SpectMorph;

using std::string;
using std::map;
using std::vector;

WavSetBuilder::WavSetBuilder (const Instrument *instrument, bool keep_samples) :
  keep_samples (keep_samples)
{
  wav_set = new WavSet();
  name = instrument->name();
  auto_volume = instrument->auto_volume();
  auto_tune = instrument->auto_tune();

  for (size_t i = 0; i < instrument->size(); i++)
    {
      Sample *sample = instrument->sample (i);
      assert (sample);

      add_sample (sample);
    }
}

WavSetBuilder::~WavSetBuilder()
{
  if (wav_set)
    {
      delete wav_set;
      wav_set = nullptr;
    }
}

void
WavSetBuilder::add_sample (const Sample *sample)
{
  SampleData sd;

  sd.midi_note = sample->midi_note();

  // FIXME: clean this up
  sd.wav_data_ptr  = const_cast<WavData *> (&sample->wav_data);

  const double clip_adjust = std::max (0.0, sample->get_marker (MARKER_CLIP_START));

  sd.loop = sample->loop();
  sd.loop_start_ms = sample->get_marker (MARKER_LOOP_START) - clip_adjust;
  sd.loop_end_ms = sample->get_marker (MARKER_LOOP_END) - clip_adjust;
  sd.clip_start_ms = sample->get_marker (MARKER_CLIP_START);
  sd.clip_end_ms = sample->get_marker (MARKER_CLIP_END);

  sample_data_vec.push_back (sd);
}

WavSet *
WavSetBuilder::run()
{
  for (auto& sd : sample_data_vec)
    {
      /* clipping */
      assert (sd.wav_data_ptr->n_channels() == 1);

      vector<float> samples = sd.wav_data_ptr->samples();
      vector<float> clipped_samples;
      for (size_t i = 0; i < samples.size(); i++)
        {
          double pos_ms = i * (1000.0 / sd.wav_data_ptr->mix_freq());
          if (pos_ms >= sd.clip_start_ms)
            {
              /* if we have a loop, the loop end determines the real end of the recording */
              if (sd.loop != Sample::Loop::NONE || pos_ms <= sd.clip_end_ms)
                clipped_samples.push_back (samples[i]);
            }
        }

      WavData wd_clipped;
      wd_clipped.load (clipped_samples, 1, sd.wav_data_ptr->mix_freq());

      WavSetWave new_wave;
      new_wave.midi_note = sd.midi_note;
      new_wave.channel = 0;
      new_wave.velocity_range_min = 0;
      new_wave.velocity_range_max = 127;
      new_wave.audio = InstEncCache::the()->encode (name, wd_clipped, sd.midi_note);

      if (keep_samples)
        new_wave.audio->original_samples = sd.wav_data_ptr->samples();

      wav_set->waves.push_back (new_wave);
    }
  apply_loop_settings();
  apply_auto_volume();
  apply_auto_tune();

  WavSet *result = wav_set;
  wav_set = nullptr;

  return result;
}

void
WavSetBuilder::apply_loop_settings()
{
  // build index for sample data vector
  map<int, SampleData*> note_to_sd;
  for (auto& sd : sample_data_vec)
    note_to_sd[sd.midi_note] = &sd;

  for (auto& wave : wav_set->waves)
    {
      SampleData *sd = note_to_sd[wave.midi_note];

      if (!sd)
        {
          printf ("warning: no to sd mapping %d failed\n", wave.midi_note);
          continue;
        }

      Audio *audio = wave.audio;

      // FIXME! account for zero_padding at start of sample
      const int loop_start = sd->loop_start_ms / audio->frame_step_ms;
      const int loop_end   = sd->loop_end_ms / audio->frame_step_ms;


      if (sd->loop == Sample::Loop::NONE)
        {
          audio->loop_type = Audio::LOOP_NONE;
          audio->loop_start = 0;
          audio->loop_end = 0;
        }
      else if (sd->loop == Sample::Loop::FORWARD)
        {
          audio->loop_type = Audio::LOOP_FRAME_FORWARD;
          audio->loop_start = loop_start;
          audio->loop_end = loop_end;
        }
      else if (sd->loop == Sample::Loop::PING_PONG)
        {
          audio->loop_type = Audio::LOOP_FRAME_PING_PONG;
          audio->loop_start = loop_start;
          audio->loop_end = loop_end;
        }
      else if (sd->loop == Sample::Loop::SINGLE_FRAME)
        {
          audio->loop_type = Audio::LOOP_FRAME_FORWARD;

          // single frame loop
          audio->loop_start = loop_start;
          audio->loop_end   = loop_start;
        }

      string lt_string;
      bool have_loop_type = Audio::loop_type_to_string (audio->loop_type, lt_string);
      if (have_loop_type)
        printf ("loop-type  = %s [%d..%d]\n", lt_string.c_str(), audio->loop_start, audio->loop_end);
    }
}

void
WavSetBuilder::apply_auto_volume()
{
  if (!auto_volume.enabled)
    return;

  for (auto& wave : wav_set->waves)
    {
      Audio& audio = *wave.audio;

      if (auto_volume.method == Instrument::AutoVolume::FROM_LOOP)
        {
          double energy = AudioTool::compute_energy (audio);

          AudioTool::normalize_energy (energy, audio);
        }
      if (auto_volume.method == Instrument::AutoVolume::GLOBAL)
        {
          AudioTool::normalize_factor (db_to_factor (auto_volume.gain), audio);
        }
    }
}

void
WavSetBuilder::apply_auto_tune()
{
  if (!auto_tune.enabled)
    return;

  for (auto& wave : wav_set->waves)
    {
      Audio& audio = *wave.audio;

      if (auto_tune.method == Instrument::AutoTune::SIMPLE)
        {
          double tune_factor;

          if (AudioTool::get_auto_tune_factor (audio, tune_factor))
            AudioTool::apply_auto_tune_factor (audio, tune_factor);
        }
      if (auto_tune.method == Instrument::AutoTune::ALL_FRAMES)
        {
          for (auto& block : audio.contents)
            {
              const double est_freq = block.estimate_fundamental (auto_tune.partials);
              const double tune_factor = 1.0 / est_freq;

              for (size_t p = 0; p < block.freqs.size(); p++)
                {
                  const double freq = block.freqs_f (p) * tune_factor;
                  block.freqs[p] = sm_freq2ifreq (freq);
                }
            }
        }
    }
}
