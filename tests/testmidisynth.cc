// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmidisynth.hh"
#include "smmain.hh"
#include "smproject.hh"
#include "smsynthinterface.hh"
#include "smmorphwavsource.hh"
#include "smmicroconf.hh"

#include <unistd.h>

using namespace SpectMorph;

using std::vector;

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);
  bool script_mode = (argc == 4) && (strcmp (argv[1], "script") == 0);
  if (argc != 3 && !script_mode)
    {
      fprintf (stderr, "usage: testmidisynth <plan> <note>\n");
      fprintf (stderr, "usage: testmidisynth script <plan> <script>\n");
      return 1;
    }

  Project project;
  project.set_mix_freq (48000);

  Error error = project.load (script_mode ? argv[2] : argv[1]);
  assert (!error);

  for (MorphOperator *op : project.morph_plan()->operators())
    {
      if (op->type_name() == "WavSource")
        {
          auto wav_source = dynamic_cast<MorphWavSource *> (op);
          while (project.rebuild_active (wav_source->object_id()))
             usleep (10 * 1000);
        }
    }
  project.try_update_synth();

  MidiSynth& midi_synth = *project.midi_synth();

  if (script_mode)
    {
      MicroConf script_parser (argv[3]);
      if (!script_parser.open_ok())
        {
          fprintf (stderr, "error opening file %s\n", argv[3]);
          exit (1);
        }
      script_parser.set_number_format (MicroConf::NO_I18N);

      while (script_parser.next())
        {
          int i, j, ch;
          double d;

          if (script_parser.command ("note_on", ch, i, j))
            {
              unsigned char note_on[3] = { uint8 (0x90 + ch), uint8 (i), uint8 (j) };
              midi_synth.add_midi_event (0, note_on);
            }
          else if (script_parser.command ("note_off", ch, i, j))
            {
              unsigned char note_off[3] = { uint8 (0x80 + ch), uint8 (i), uint8 (j) };
              midi_synth.add_midi_event (0, note_off);
            }
          else if (script_parser.command ("process", i))
            {
              vector<float> output (i);
              midi_synth.process (output.data(), i);
              for (auto f: output)
                sm_printf ("%.17g\n", f);
            }
          else if (script_parser.command ("control", i, d))
            {
              midi_synth.set_control_input (i, d);
            }
          else if (script_parser.command ("pitch_expression", ch, i, d))
            {
              midi_synth.add_pitch_expression_event (0, d, ch, i);
            }
          else
            {
              script_parser.die_if_unknown();
            }
        }
    }
  else
    {
      const unsigned char note = atoi (argv[2]);
      vector<float> output (24000);

      unsigned char note_on[3] = { 0x90, note, 100 };
      midi_synth.add_midi_event (0, note_on);
      midi_synth.process (output.data(), output.size());
      for (auto f: output)
        sm_printf ("%.17g\n", f);

      unsigned char note_off[3] = { 0x80, note, 0 };
      midi_synth.add_midi_event (0, note_off);
      while (midi_synth.active_voice_count() > 0)
        {
          midi_synth.process (output.data(), output.size());
          for (auto f: output)
            sm_printf ("%.17g\n", f);
        }
    }
}
