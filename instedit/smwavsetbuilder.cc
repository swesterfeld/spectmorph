// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavsetbuilder.hh"

using namespace SpectMorph;

using std::string;

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

  sample_data_vec.push_back (sd);
}

void
WavSetBuilder::run()
{
  WavSet wav_set;

  for (auto& sd : sample_data_vec)
    {
      string sm_name = string_printf ("/tmp/x%d.sm", sd.midi_note);

      sd.wav_data_ptr->save ("/tmp/x.wav");
      string cmd = string_printf ("smenccache /tmp/x.wav %s -m %d -O1 -s", sm_name.c_str(), sd.midi_note);
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
}

void
WavSetBuilder::get_result (WavSet& wav_set)
{
  wav_set.load ("/tmp/x.smset");
}
