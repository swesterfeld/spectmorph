// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavsetbuilder.hh"

using namespace SpectMorph;

using std::string;
using std::map;
using std::vector;

WavSetBuilder::WavSetBuilder (const Instrument *instrument)
{
  for (size_t i = 0; i < instrument->size(); i++)
    {
      Sample *sample = instrument->sample (i);
      assert (sample);

      add_sample (sample);
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

void
WavSetBuilder::run()
{
  WavSet wav_set;

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
      wd_clipped.save ("/tmp/x.wav");

      /* encoding */
      string sm_name = string_printf ("/tmp/x%d.sm", sd.midi_note);

      string cmd = string_printf ("%s/smenccache /tmp/x.wav %s -m %d -O1 -s", sm_get_install_dir (INSTALL_DIR_BIN).c_str(), sm_name.c_str(), sd.midi_note);
      printf ("# %s\n", cmd.c_str());
      system (cmd.c_str());

      WavSetWave new_wave;
      new_wave.midi_note = sd.midi_note;
      new_wave.path = sm_name;
      new_wave.channel = 0;
      new_wave.velocity_range_min = 0;
      new_wave.velocity_range_max = 127;

      wav_set.waves.push_back (new_wave);
    }
  wav_set.save ("/tmp/x.smset", true); // link wavset

  apply_loop_settings();
}

void
WavSetBuilder::apply_loop_settings()
{
  WavSet wav_set;

  wav_set.load ("/tmp/x.smset");

  // build index for sample data vector
  map<int, SampleData*> note_to_sd;
  for (auto& sd : sample_data_vec)
    note_to_sd[sd.midi_note] = &sd;

  for (auto& wave : wav_set.waves)
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

  wav_set.save ("/tmp/x.smset");
}

void
WavSetBuilder::get_result (WavSet& wav_set)
{
  wav_set.load ("/tmp/x.smset");
}
