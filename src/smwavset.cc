/* 
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>

#include "config.h"
#include <smwavset.hh>
#include <smaudio.hh>
#include <bse/bsemain.h>
#include <bse/bseloader.h>

#include <string>
#include <map>

using std::string;
using std::vector;
using std::map;

using namespace SpectMorph;

/// @cond
struct Options
{
  string	program_name; /* FIXME: what to do with that */
  string        data_dir;
  string        args;
  enum { NONE, INIT, ADD, LIST, ENCODE, DECODE, DELTA, LINK } command;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

Options::Options ()
{
  program_name = "smwavset";
  data_dir = "/tmp";
  args = "";
  command = NONE;
}

#include "stwutils.hh"

void
Options::parse (int   *argc_p,
                char **argv_p[])
{
  guint argc = *argc_p;
  gchar **argv = *argv_p;
  unsigned int i, e;

  g_return_if_fail (argc >= 0);

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
	  printf ("%s %s\n", program_name.c_str(), VERSION);
	  exit (0);
	}
      else if (check_arg (argc, argv, &i, "--args", &opt_arg))
        {
          args = opt_arg;
        }
      else if (check_arg (argc, argv, &i, "-d", &opt_arg) ||
               check_arg (argc, argv, &i, "--data-dir", &opt_arg))
	{
	  data_dir = opt_arg;
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
  printf ("usage: %s <command> [ <options> ] [ <command specific args...> ]\n", options.program_name.c_str());
  printf ("\n");
  printf ("command specific args:\n");
  printf ("\n");
  printf (" smwavset init [ <options> ] <wset_filename>...\n");
  printf (" smwavset add [ <options> ] <wset_filename> <midi_note> <path>\n");
  printf (" smwavset list [ <options> ] <wset_filename>\n");
  printf (" smwavset encode [ <options> ] <wset_filename> <smset_filename>\n");
  printf (" smwavset decode [ <options> ] <smset_filename> <wset_filename>\n");
  printf (" smwavset delta [ <options> ] <wset_filename1>...<wset_filenameN>\n");
  printf ("\n");
  printf ("options:\n");
  printf (" -h, --help                  help for %s\n", options.program_name.c_str());
  printf (" -v, --version               print version\n");
  printf (" --args \"<args>\"             arguments for decoder or encoder\n");
  printf (" -d, --data-dir <dir>        set data directory for newly created .sm or .wav files\n");
  printf ("\n");
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

double
gettime ()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
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
  BseErrorType error;

  BseWaveFileInfo *wave_file_info = bse_wave_file_info_load (filename.c_str(), &error);
  if (!wave_file_info)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      return false;
    }

  BseWaveDsc *waveDsc = bse_wave_dsc_load (wave_file_info, 0, FALSE, &error);
  if (!waveDsc)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      return false;
    }

  GslDataHandle *dhandle = bse_wave_handle_create (waveDsc, 0, &error);
  if (!dhandle)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      return false;
    }

  error = gsl_data_handle_open (dhandle);
  if (error)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      return false;
    }

  if (gsl_data_handle_n_channels (dhandle) != 1)
    {
      fprintf (stderr, "Currently, only mono files are supported.\n");
      return false;
    }

  data_out.clear();

  vector<float> block (1024);

  const uint64 n_values = gsl_data_handle_length (dhandle);
  for (uint64 pos = 0; pos < n_values; pos += block.size())
    {
      /* read data from file */
      uint64 r = gsl_data_handle_read (dhandle, pos, block.size(), &block[0]);

      for (uint64 t = 0; t < r; t++)
        data_out.push_back (block[t]);
    }
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
  BseErrorType error = wset.load (name);
  if (error)
    {
      fprintf (stderr, "%s: can't open input file: %s: %s\n", options.program_name.c_str(), name.c_str(), bse_error_blurb (error));
      exit (1);
    }
}

int
main (int argc, char **argv)
{
  double start_time = gettime();

  bse_init_inprocess (&argc, &argv, NULL, NULL);

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

      wset.waves.push_back (new_wave);
      wset.save (argv[1]);
    }
  else if (options.command == Options::LIST)
    {
      WavSet wset;
      load_or_die (wset, argv[1]);

      for (vector<WavSetWave>::iterator wi = wset.waves.begin(); wi != wset.waves.end(); wi++)
        printf ("%d %s\n", wi->midi_note, wi->path.c_str());
    }
  else if (options.command == Options::ENCODE)
    {
      assert (argc == 3);

      WavSet wset, smset;
      load_or_die (wset, argv[1]);

      for (vector<WavSetWave>::iterator wi = wset.waves.begin(); wi != wset.waves.end(); wi++)
        {
          string smpath = options.data_dir + "/" + int2str (wi->midi_note) + ".sm";
          string cmd = "smenc -m " + int2str (wi->midi_note) + " " + wi->path.c_str() + " " + smpath + " " + options.args;
          printf ("[%s] ## %s\n", time2str (gettime() - start_time).c_str(), cmd.c_str());
          system (cmd.c_str());

          WavSetWave new_wave = *wi;
          new_wave.path = smpath;
          smset.waves.push_back (new_wave);
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
          if (audio.load (si->path, AUDIO_SKIP_DEBUG) != BSE_ERROR_NONE)
            {
              printf ("can't load %s\n", si->path.c_str());
              exit (1);
            }

          string wavpath = options.data_dir + "/" + int2str (si->midi_note) + ".wav";
          string cmd = "smplay --rate=" + int2str (audio.mix_freq) + " " +si->path.c_str() + " --export " + wavpath + " " + options.args;
          printf ("[%s] ## %s\n", time2str (gettime() - start_time).c_str(), cmd.c_str());
          system (cmd.c_str());

          WavSetWave new_wave = *si;
          new_wave.path = wavpath;
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
              double ref_delta;

              printf ("%3d: ", i);
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
                      printf ("%3.4f%% ", 100 * this_delta / ref_delta);
                    }
                  else
                    {
                      printf ("an error occured during loading the files.\n");
                      exit (1);
                    }
                }
              printf ("\n");
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
  else
    {
      printf ("You need to specify a command (init, add, list, encode, decode, delta).\n\n");
      Options::print_usage();
      exit (1);
    }
}
