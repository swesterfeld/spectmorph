// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmidisynth.hh"
#include "smmain.hh"
#include "smproject.hh"
#include "smsynthinterface.hh"

using namespace SpectMorph;

using std::vector;

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);
  if (argc != 3)
    {
      fprintf (stderr, "usage: testmidisynth <plan> <note>\n");
      return 1;
    }

  Project project;
  project.set_mix_freq (48000);

  Error error = project.load (argv[1]);
  assert (!error);

  project.try_update_synth();

  MidiSynth& midi_synth = *project.midi_synth();

  const unsigned char note = atoi (argv[2]);
  vector<float> output (24000);

  unsigned char note_on[3] = { 0x90, note, 100 };
  midi_synth.add_midi_event (0, note_on);
  midi_synth.process (&output[0], output.size());
  for (auto f: output)
    sm_printf ("%.17g\n", f);

  unsigned char note_off[3] = { 0x80, note, 0 };
  midi_synth.add_midi_event (0, note_off);
  while (midi_synth.active_voice_count() > 0)
    {
      midi_synth.process (&output[0], output.size());
      for (auto f: output)
        sm_printf ("%.17g\n", f);
    }
}
