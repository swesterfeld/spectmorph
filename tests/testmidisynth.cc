// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmidisynth.hh"
#include "smmain.hh"

using namespace SpectMorph;

using std::vector;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);
  if (argc != 3)
    {
      fprintf (stderr, "usage: testmidisynth <plan> <note>\n");
      return 1;
    }

  MidiSynth midi_synth (48000, 16 /* voices */);

  MorphPlanPtr plan (new MorphPlan());

  GenericIn *in = StdioIn::open (argv[1]);
  if (!in)
    {
      g_printerr ("Error opening '%s'.\n", argv[1]);
      exit (1);
    }
  plan->load (in);
  delete in;

  midi_synth.update_plan (plan);

  const unsigned char note = atoi (argv[2]);
  vector<float> output (24000);

  unsigned char note_on[3] = { 0x90, note, 100 };
  midi_synth.add_midi_event (0, note_on);
  midi_synth.process (&output[0], output.size());
  for (auto f: output)
    printf ("%.17g\n", f);

  unsigned char note_off[3] = { 0x80, note, 0 };
  midi_synth.add_midi_event (0, note_off);
  while (midi_synth.active_voice_count() > 0)
    {
      midi_synth.process (&output[0], output.size());
      for (auto f: output)
        printf ("%.17g\n", f);
    }
}
