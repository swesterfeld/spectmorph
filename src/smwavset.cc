// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "config.h"
#include <smwavset.hh>
#include <smaudio.hh>
#include <smmain.hh>
#include <smmicroconf.hh>
#include "smjobqueue.hh"
#include "smutils.hh"
#include "smwavdata.hh"
#include "sminstrument.hh"
#include "smwavsetbuilder.hh"

#include <string>
#include <map>

using std::string;
using std::vector;
using std::map;

using namespace SpectMorph;

static vector<string>
string_tokenize (const string& str)
{
  vector<string> words;

  string::const_iterator word_start = str.begin();
  for (string::const_iterator si = str.begin(); si != str.end(); si++)
    {
      if (*si == ',') /* colon indicates word boundary */
        {
          words.push_back (string (word_start, si));
          word_start = si + 1;
        }
    }

  if (!str.empty()) /* handle last word in string */
    words.push_back (string (word_start, str.end()));

  return words;
}

/// @cond
struct Options
{
  string	  program_name; /* FIXME: what to do with that */
  string          data_dir;
  string          args;
  string          smenc;
  int             channel;
  int             min_velocity;
  int             max_velocity;
  vector<string>  format;
  int             max_jobs;
  bool            loop_markers = false;
  bool            loop_markers_ms = false;
  enum { NONE, INIT, ADD, LIST, ENCODE, DECODE, DELTA, LINK, EXTRACT, GET_MARKERS, SET_MARKERS, SET_NAMES, GET_NAMES, BUILD } command;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

Options::Options ()
{
  program_name = "smwavset";
  smenc = "smenc";
  data_dir = "/tmp";
  args = "";
  command = NONE;
  channel = 0;
  min_velocity = 0;
  max_velocity = 127;
  format = string_tokenize ("midi-note,filename");
  max_jobs = 1;
  loop_markers = false;
}

#include "stwutils.hh"

void
Options::parse (int   *argc_p,
                char **argv_p[])
{
  guint argc = *argc_p;
  gchar **argv = *argv_p;
  unsigned int i, e;

  /*  I am tired of seeing .libs/lt-gst123 all the time,
   *  but basically this should be done (to allow renaming the binary):
   *
  if (argc && argv[0])
    program_name = argv[0];
  */

  for (i = 1; i < argc; i++)
    {
      const char *opt_arg;
      if (strcmp (argv[i], "--help") == 0 ||
          strcmp (argv[i], "-h") == 0)
	{
	  print_usage();
	  exit (0);
	}
      else if (strcmp (argv[i], "--version") == 0 || strcmp (argv[i], "-v") == 0)
	{
	  sm_printf ("%s %s\n", program_name.c_str(), VERSION);
	  exit (0);
	}
      else if (check_arg (argc, argv, &i, "--args", &opt_arg))
        {
          args = opt_arg;
        }
      else if (check_arg (argc, argv, &i, "--smenc", &opt_arg))
        {
          smenc = opt_arg;
        }
      else if (check_arg (argc, argv, &i, "-d", &opt_arg) ||
               check_arg (argc, argv, &i, "--data-dir", &opt_arg))
	{
	  data_dir = opt_arg;
        }
      else if (check_arg (argc, argv, &i, "-c", &opt_arg) ||
               check_arg (argc, argv, &i, "--channel", &opt_arg))
	{
	  channel = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--min-velocity", &opt_arg))
	{
	  min_velocity = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--max-velocity", &opt_arg))
	{
	  max_velocity = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--format", &opt_arg))
        {
          format = string_tokenize (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "-j", &opt_arg))
        {
          max_jobs = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--loop"))
        {
          loop_markers = true;
        }
      else if (check_arg (argc, argv, &i, "--loop-ms"))
        {
          loop_markers_ms = true;
        }
    }

  bool resort_required = true;

  while (resort_required)
    {
      /* resort argc/argv */
      e = 1;
      for (i = 1; i < argc; i++)
        if (argv[i])
          {
            argv[e++] = argv[i];
            if (i >= e)
              argv[i] = NULL;
          }
      *argc_p = e;
      resort_required = false;

      // parse command
      if (*argc_p >= 2 && command == NONE)
        {
          if (strcmp (argv[1], "init") == 0)
            {
              command = INIT;
            }
          else if (strcmp (argv[1], "add") == 0)
            {
              command = ADD;
            }
          else if (strcmp (argv[1], "list") == 0)
            {
              command = LIST;
            }
          else if (strcmp (argv[1], "encode") == 0)
            {
              command = ENCODE;
            }
          else if (strcmp (argv[1], "decode") == 0)
            {
              command = DECODE;
            }
          else if (strcmp (argv[1], "delta") == 0)
            {
              command = DELTA;
            }
          else if (strcmp (argv[1], "link") == 0)
            {
              command = LINK;
            }
          else if (strcmp (argv[1], "extract") == 0)
            {
              command = EXTRACT;
            }
          else if (strcmp (argv[1], "get-markers") == 0)
            {
              command = GET_MARKERS;
            }
          else if (strcmp (argv[1], "set-markers") == 0)
            {
              command = SET_MARKERS;
            }
          else if (strcmp (argv[1], "set-names") == 0)
            {
              command = SET_NAMES;
            }
          else if (strcmp (argv[1], "get-names") == 0)
            {
              command = GET_NAMES;
            }
          else if (strcmp (argv[1], "build") == 0)
            {
              command = BUILD;
            }

          if (command != NONE)
            {
              argv[1] = NULL;
              resort_required = true;
            }
        }
    }
}

void
Options::print_usage ()
{
  sm_printf ("usage: %s <command> [ <options> ] [ <command specific args...> ]\n", options.program_name.c_str());
  sm_printf ("\n");
  sm_printf ("command specific args:\n");
  sm_printf ("\n");
  sm_printf (" smwavset init [ <options> ] <wset_filename>...\n");
  sm_printf (" smwavset add [ <options> ] <wset_filename> <midi_note> <path>\n");
  sm_printf (" smwavset list [ <options> ] <wset_filename>\n");
  sm_printf (" smwavset encode [ <options> ] <wset_filename> <smset_filename>\n");
  sm_printf (" smwavset decode [ <options> ] <smset_filename> <wset_filename>\n");
  sm_printf (" smwavset delta [ <options> ] <wset_filename1>...<wset_filenameN>\n");
  sm_printf (" smwavset link [ <options> ] <wset_filename>\n");
  sm_printf (" smwavset get-markers [ <options> ] <wset_filename>\n");
  sm_printf (" smwavset set-markers [ <options> ] <wset_filename> <marker_filename>\n");
  sm_printf (" smwavset build <inst_filename> <smset_filename>\n");
  sm_printf ("\n");
  sm_printf ("options:\n");
  sm_printf (" -h, --help                  help for %s\n", options.program_name.c_str());
  sm_printf (" -v, --version               print version\n");
  sm_printf (" --args \"<args>\"             arguments for decoder or encoder\n");
  sm_printf (" -d, --data-dir <dir>        set data directory for newly created .sm or .wav files\n");
  sm_printf (" -c, --channel <ch>          set channel for added .sm file\n");
  sm_printf (" --format <f1>,...,<fN>      set fields to display in list\n");
  sm_printf (" -j <jobs>                   run <jobs> commands simultaneously (use multiple cpus for encoding)\n");
  sm_printf (" --smenc <cmd>               use <cmd> as smenc command\n");
  sm_printf (" --loop                      also extract loop markers (for smwavset get-markers)\n");
  sm_printf ("\n");
}

string
int2str (int i)
{
  char buffer[512];
  sprintf (buffer, "%d", i);
  return buffer;
}

vector<WavSetWave>::iterator
find_wave (WavSet& wset, int midi_note)
{
  vector<WavSetWave>::iterator wi = wset.waves.begin(); 

  while (wi != wset.waves.end())
    {
      if (wi->midi_note == midi_note)
        return wi;
      wi++;
    }

  return wi;
}

string
time2str (double t)
{
  long long ms = t * 1000;
  long long s = ms / 1000;
  long long m = s / 60;
  long long h = m / 60;

  char buf[1024];
  sprintf (buf, "%02lld:%02lld:%02lld.%03lld", h, m % 60, s % 60, ms % 1000);
  return buf;
}

bool
load_wav_file (const string& filename, vector<float>& data_out)
{
  /* open input */
  WavData wav_data;
  if (!wav_data.load_mono (filename))
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), wav_data.error_blurb());
      return false;
    }
  data_out = wav_data.samples();
  return true;
}

double
delta (vector<float>& d0, vector<float>& d1)
{
  double error = 0;
  for (size_t t = 0; t < MAX (d0.size(), d1.size()); t++)
    {
      double a0 = 0, a1 = 0;
      if (t < d0.size())
        a0 = d0[t];
      if (t < d1.size())
        a1 = d1[t];
      error += (a0 - a1) * (a0 - a1);
    }
  return error / MAX (d0.size(), d1.size());
}

void
load_or_die (WavSet& wset, string name)
{
  Error error = wset.load (name);
  if (error)
    {
      fprintf (stderr, "%s: can't open input file: %s: %s\n", options.program_name.c_str(), name.c_str(), error.message());
      exit (1);
    }
}

int
main (int argc, char **argv)
{
  double start_time = get_time();

  Main main (&argc, &argv);

  options.parse (&argc, &argv);

  if (options.command == Options::INIT)
    {
      for (int i = 1; i < argc; i++)
        {
          WavSet wset;
          wset.save (argv[i]);
        }
    }
  else if (options.command == Options::ADD)
    {
      assert (argc == 4);

      WavSet wset;
      load_or_die (wset, argv[1]);

      WavSetWave new_wave;
      new_wave.midi_note = atoi (argv[2]);
      new_wave.path = argv[3];
      new_wave.channel = options.channel;
      new_wave.velocity_range_min = options.min_velocity;
      new_wave.velocity_range_max = options.max_velocity;

      wset.waves.push_back (new_wave);
      wset.save (argv[1]);
    }
  else if (options.command == Options::LIST)
    {
      WavSet wset;
      load_or_die (wset, argv[1]);

      for (vector<WavSetWave>::iterator wi = wset.waves.begin(); wi != wset.waves.end(); wi++)
        {
          for (vector<string>::const_iterator fi = options.format.begin(); fi != options.format.end(); fi++)
            {
              if (fi != options.format.begin()) // not first field
                sm_printf (" ");
              if (*fi == "midi-note")
                sm_printf ("%d", wi->midi_note);
              else if (*fi == "filename")
                sm_printf ("%s", wi->path.c_str());
              else if (*fi == "channel")
                sm_printf ("%d", wi->channel);
              else if (*fi == "min-velocity")
                sm_printf ("%d", wi->velocity_range_min);
              else if (*fi == "max-velocity")
                sm_printf ("%d", wi->velocity_range_max);
              else
                {
                  fprintf (stderr, "list command: invalid field for format: %s\n", fi->c_str());
                  exit (1);
                }
            }
          sm_printf ("\n");
        }
    }
  else if (options.command == Options::SET_NAMES)
    {
      assert (argc == 4);

      WavSet wset;
      load_or_die (wset, argv[1]);
      wset.name = argv[2];
      wset.short_name = argv[3];
      wset.save (argv[1]);
    }
  else if (options.command == Options::GET_NAMES)
    {
      assert (argc == 2);

      WavSet wset;
      load_or_die (wset, argv[1]);
      sm_printf ("name \"%s\"\n", wset.name.c_str());
      sm_printf ("short_name \"%s\"\n", wset.short_name.c_str());
    }
  else if (options.command == Options::ENCODE)
    {
      assert (argc == 3);

      WavSet wset, smset;
      load_or_die (wset, argv[1]);

      JobQueue job_queue (options.max_jobs);

      for (vector<WavSetWave>::iterator wi = wset.waves.begin(); wi != wset.waves.end(); wi++)
        {
          string smpath = options.data_dir + "/" + int2str (wi->midi_note) + ".sm";
          string cmd = options.smenc + " -m " + int2str (wi->midi_note) + " \"" + wi->path.c_str() + "\" " + smpath + " " + options.args;
          sm_printf ("[%s] ## %s\n", time2str (get_time() - start_time).c_str(), cmd.c_str());
          job_queue.run (cmd);

          WavSetWave new_wave = *wi;
          new_wave.path = smpath;
          smset.waves.push_back (new_wave);
        }
      if (!job_queue.wait_for_all())
        {
          g_printerr ("smwavset: encoding commands did not complete successfully\n");
          exit (1);
        }
      smset.save (argv[2]);
    }
  else if (options.command == Options::DECODE)
    {
      assert (argc == 3);

      WavSet smset, wset;
      load_or_die (smset, argv[1]);

      for (vector<WavSetWave>::const_iterator si = smset.waves.begin(); si != smset.waves.end(); si++)
        {
          Audio audio;
          if (!audio.load (si->path, AUDIO_SKIP_DEBUG))
            {
              sm_printf ("can't load %s\n", si->path.c_str());
              exit (1);
            }

          string wavpath = options.data_dir + "/" + int2str (si->midi_note) + ".wav";
          string cmd = "smplay --rate=" + int2str (audio.mix_freq) + " " +si->path.c_str() + " --export " + wavpath + " " + options.args;
          sm_printf ("[%s] ## %s\n", time2str (get_time() - start_time).c_str(), cmd.c_str());
          int rc = system (cmd.c_str());
          if (rc != 0)
            {
              printf ("command execution failed: %d\n", WEXITSTATUS (rc));
              exit (1);
            }

          WavSetWave new_wave = *si;
          new_wave.path = wavpath;
          new_wave.audio = NULL;  // without this, Audio destructor will be run twice
          wset.waves.push_back (new_wave);
        }
      wset.save (argv[2]);
    }
  else if (options.command == Options::DELTA)
    {
      assert (argc >= 2);

      vector<WavSet> wsets;
      map<int,bool>              midi_note_used;

      for (int i = 1; i < argc; i++)
        {
          WavSet wset;
          load_or_die (wset, argv[i]);
          wsets.push_back (wset);
          for (vector<WavSetWave>::const_iterator wi = wset.waves.begin(); wi != wset.waves.end(); wi++)
            midi_note_used[wi->midi_note] = true;              
        }
      for (int i = 0; i < 128; i++)
        {
          if (midi_note_used[i])
            {
              double ref_delta = -1; /* init to get rid of gcc warning */

              sm_printf ("%3d: ", i);
              vector<WavSetWave>::iterator w0 = find_wave (wsets[0], i);
              assert (w0 != wsets[0].waves.end());
              for (size_t w = 1; w < wsets.size(); w++)
                {
                  vector<WavSetWave>::iterator w1 = find_wave (wsets[w], i);
                  assert (w1 != wsets[w].waves.end());

                  vector<float> data0, data1;
                  if (load_wav_file (w0->path, data0) && load_wav_file (w1->path, data1))
                    {
                      double this_delta = delta (data0, data1);
                      if (w == 1)
                        ref_delta = this_delta;
                      sm_printf ("%3.4f%% ", 100 * this_delta / ref_delta);
                    }
                  else
                    {
                      sm_printf ("an error occured during loading the files.\n");
                      exit (1);
                    }
                }
              sm_printf ("\n");
            }
        }
    }
  else if (options.command == Options::LINK)
    {
      assert (argc == 2);

      WavSet wset;
      load_or_die (wset, argv[1]);
      wset.save (argv[1], true);
    }
  else if (options.command == Options::EXTRACT)
    {
      assert (argc == 3);
      WavSet wset;
      load_or_die (wset, argv[1]);
      for (vector<WavSetWave>::const_iterator wi = wset.waves.begin(); wi != wset.waves.end(); wi++)
        if (wi->path == argv[2])
          {
            wi->audio->save (argv[2]);
            return 0;
          }
      fprintf (stderr, "error: path %s not found\n", argv[2]);
      exit (1);
    }
  else if (options.command == Options::GET_MARKERS)
    {
      assert (argc == 2);

      WavSet wset;
      load_or_die (wset, argv[1]);

      sm_printf ("# smwavset markers for %s\n", argv[1]);
      sm_printf ("\n");

      for (vector<WavSetWave>::const_iterator wi = wset.waves.begin(); wi != wset.waves.end(); wi++)
        {
          string loop_type;
          if (!Audio::loop_type_to_string (wi->audio->loop_type, loop_type))
            {
              g_printerr ("smwavset: can't convert loop type to string");
              exit (1);
            }
          if (options.loop_markers)
            {
              sm_printf ("set-marker loop-type %d %d %d %d %s\n", wi->midi_note, wi->channel,
                wi->velocity_range_min, wi->velocity_range_max, loop_type.c_str());
              sm_printf ("set-marker loop-start %d %d %d %d %d\n", wi->midi_note, wi->channel,
                wi->velocity_range_min, wi->velocity_range_max, wi->audio->loop_start);
              sm_printf ("set-marker loop-end %d %d %d %d %d\n", wi->midi_note, wi->channel,
                wi->velocity_range_min, wi->velocity_range_max, wi->audio->loop_end);
            }
          if (options.loop_markers_ms)
            {
              const double zero_values_ms = wi->audio->zero_values_at_start / wi->audio->mix_freq * 1000.0;
              const double loop_start     = wi->audio->loop_start * wi->audio->frame_step_ms - zero_values_ms;
              const double loop_end       = wi->audio->loop_end * wi->audio->frame_step_ms - zero_values_ms;

              sm_printf ("set-loop-ms %d %s %f %f\n", wi->midi_note, loop_type.c_str(), loop_start, loop_end);
            }
        }
    }
  else if (options.command == Options::SET_MARKERS)
    {
      assert (argc == 3);

      WavSet wset;
      load_or_die (wset, argv[1]);

      MicroConf cfg (argv[2]);
      while (cfg.next())
        {
          string marker_type;
          int midi_note, channel, vmin, vmax;
          string arg;

          if (cfg.command ("set-marker", marker_type, midi_note, channel, vmin, vmax, arg))
            {
              bool match = false;
              for (vector<WavSetWave>::const_iterator wi = wset.waves.begin(); wi != wset.waves.end(); wi++)
                {
                  if ((wi->midi_note == midi_note)
                  &&  (wi->channel   == channel)
                  &&  (wi->velocity_range_min == vmin)
                  &&  (wi->velocity_range_max == vmax))
                    {
                      if (marker_type == "start")
                        {
                          fprintf (stderr, "smwavset: ignoring no longer supported marker start\n");
                          match = true;
                        }
                      else if (marker_type == "loop-type")
                        {
                          Audio::LoopType loop_type;

                          if (Audio::string_to_loop_type (arg, loop_type))
                            {
                              wi->audio->loop_type = loop_type;
                              match = true;
                            }
                          else
                            {
                              g_printerr ("smwavset: unknown loop_type %s\n", arg.c_str());
                              exit (1);
                            }
                        }
                      else if (marker_type == "loop-start")
                        {
                          wi->audio->loop_start = atoi (arg.c_str());
                          match = true;
                        }
                      else if (marker_type == "loop-end")
                        {
                          wi->audio->loop_end = atoi (arg.c_str());
                          match = true;
                        }
                    }
                }
              if (!match)
                {
                  sm_printf ("no match for marker %s %d %d %d %d %s\n", marker_type.c_str(), midi_note, channel,
                             vmin, vmax, arg.c_str());
                  exit (1);
                }
            }
          else
            {
              cfg.die_if_unknown();
            }
        }
      wset.save (argv[1]);
    }
  else if (options.command == Options::BUILD)
    {
      assert (argc == 3);

      string inst_filename = argv[1];
      string out_filename = argv[2];

      Instrument inst;
      inst.load (inst_filename);

      WavSetBuilder builder (&inst, /* keep_samples */ false);
      std::unique_ptr<WavSet> smset (builder.run());
      assert (smset);

      smset->save (out_filename);
    }
  else
    {
      sm_printf ("You need to specify a command (init, add, list, encode, decode, delta, build).\n\n");
      Options::print_usage();
      exit (1);
    }
}
