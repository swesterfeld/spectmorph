// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "smnoisedecoder.hh"
#include "smsinedecoder.hh"
#include "sminfile.hh"
#include "smwavset.hh"
#include "smmain.hh"
#include "smwavdata.hh"
#include "smaudio.hh"
#include "config.h"

#if SPECTMORPH_HAVE_AO
#include <ao/ao.h>
#endif

#include <list>

using namespace SpectMorph;
using std::max;
using std::string;
using std::vector;

/// @cond
struct Options
{
  string              program_name  = "smplay";
  SineDecoder::Mode   decoder_mode  = SineDecoder::MODE_PHASE_SYNC_OVERLAP;
  bool                noise_enabled = true;
  bool                sines_enabled = true;
  bool                deterministic_random = false;
  int                 rate          = 44100;
  int                 bits          = 16;
  int                 midi_note     = -1;
  double              gain          = 1.0;
  string              export_wav;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

#include "stwutils.hh"

Options::Options ()
{
}

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
	  printf ("%s %s\n", program_name.c_str(), VERSION);
	  exit (0);
	}
      else if (check_arg (argc, argv, &i, "--rate", &opt_arg) || check_arg (argc, argv, &i, "-r", &opt_arg))
	{
          if (!sm_try_atoi (opt_arg, rate) || rate < 8000)
            {
              fprintf (stderr, "%s: invalid rate '%s', should be integer >= 8000\n", options.program_name.c_str(), opt_arg);
              exit (1);
            }
	}
      else if (check_arg (argc, argv, &i, "--bits", &opt_arg))
        {
          if (!sm_try_atoi (opt_arg, bits) || bits < 8)
            {
              fprintf (stderr, "%s: invalid bit depth '%s', should be integer >= 8\n", options.program_name.c_str(), opt_arg);
              exit (1);
            }
        }
      else if (check_arg (argc, argv, &i, "--export", &opt_arg) || check_arg (argc, argv, &i, "-x", &opt_arg))
        {
          export_wav = opt_arg;
        }
      else if (check_arg (argc, argv, &i, "--midi-note", &opt_arg) || check_arg (argc, argv, &i, "-m", &opt_arg))
        {
          if (!sm_try_atoi (opt_arg, midi_note) || midi_note < 0 || midi_note > 127)
            {
              fprintf (stderr, "%s: invalid midi note '%s', should be integer between 0 and 127\n", options.program_name.c_str(), opt_arg);
              exit (1);
            }
        }
      else if (check_arg (argc, argv, &i, "--gain", &opt_arg) || check_arg (argc, argv, &i, "-g", &opt_arg))
        {
          gain = sm_atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--no-noise"))
        {
          noise_enabled = false;
        }
      else if (check_arg (argc, argv, &i, "--no-sines"))
        {
          sines_enabled = false;
        }
      else if (check_arg (argc, argv, &i, "--det-random"))
        {
          deterministic_random = true;
        }
      else if (check_arg (argc, argv, &i, "--decoder-mode", &opt_arg) || check_arg (argc, argv, &i, "-M", &opt_arg))
        {
          if (strcmp (opt_arg, "phase-sync") == 0)
            decoder_mode = SineDecoder::MODE_PHASE_SYNC_OVERLAP;
          else if (strcmp (opt_arg, "tracking") == 0)
            decoder_mode = SineDecoder::MODE_TRACKING;
          else
            {
              g_printerr ("unknown decoder mode: %s\n", opt_arg);
              exit (1);
            }
        }
    }

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
}

void
Options::print_usage ()
{
  printf ("usage: %s [ <options> ] <sm_file>\n", options.program_name.c_str());
  printf ("\n");
  printf ("options:\n");
  printf (" -h, --help                  help for %s\n", options.program_name.c_str());
  printf (" -v, --version               print version\n");
  printf (" --rate <sampling rate>      set replay rate manually\n");
  printf (" --no-noise                  disable noise decoder\n");
  printf (" --no-sines                  disable sine decoder\n");
  printf (" --det-random                use deterministic/reproducable random generator\n");
  printf (" --export <wav filename>     export to wav file\n");
  printf (" --bits <bits>               set export bit depth\n");
  printf (" -m, --midi-note <note>      set midi note to play for wavsets\n");
  printf ("\n");
}

int
main (int argc, char **argv)
{
  /* init */
  Main main (&argc, &argv);
  options.parse (&argc, &argv);
  if (argc != 2)
    {
      options.print_usage();
      exit (1);
    }

  /* open input */

  /* figure out file type (we support SpectMorph::WavSet and SpectMorph::Audio) */
  InFile *file = new InFile (argv[1]);
  if (!file->open_ok())
    {
      fprintf (stderr, "%s: can't open input file: %s\n", argv[0], argv[1]);
      exit (1);
    }
  string file_type = file->file_type();
  delete file;

  SpectMorph::Audio *audio_ptr = NULL;

  // only one of these two will be used, depending on the file_type
  SpectMorph::WavSet wset;                  // for audio object within wavset playback
  SpectMorph::Audio  audio_to_play;         // for single audio object playback

  if (file_type == "SpectMorph::WavSet")    // load wavset
    {
      Error error = wset.load (argv[1]);
      if (error)
        {
          fprintf (stderr, "%s: can't open input file: %s: %s\n", argv[0], argv[1], error.message());
          exit (1);
        }
      for (vector<WavSetWave>::iterator wi = wset.waves.begin(); wi != wset.waves.end(); wi++)
        {
          if (wi->midi_note == options.midi_note)
            audio_ptr = wi->audio;
        }
      if (audio_ptr == NULL)
        {
          fprintf (stderr, "%s: wavset %s does not contain midi note %d\n", argv[0], argv[1], options.midi_note);
          exit (1);
        }
    }
  else                                     // load single audio file
    {
      audio_ptr = &audio_to_play;
      Error error = audio_ptr->load (argv[1], AUDIO_SKIP_DEBUG);
      if (error)
        {
          fprintf (stderr, "%s: can't open input file: %s: %s\n", argv[0], argv[1], error.message());
          exit (1);
        }
    }

  SpectMorph::Audio& audio = *audio_ptr;

#if SPECTMORPH_HAVE_AO
  ao_device *play_device = NULL;

  if (options.export_wav == "")   /* open audio only if we need to play something */
    {
      ao_sample_format format = { 0, };

      format.bits = 16;
      format.rate = options.rate;
      format.channels = 1;
      format.byte_format = AO_FMT_NATIVE;

      ao_initialize();

      ao_option *ao_options = NULL;
      int driver_id = ao_default_driver_id ();
      if ((play_device = ao_open_live (driver_id, &format, ao_options)) == NULL)
        {
          fprintf (stderr, "%s: can't open oss output: %s\n", argv[0], strerror (errno));
          exit(1);
        }
    }
#endif

  size_t frame_size = audio.frame_size_ms * options.rate / 1000;
  size_t frame_step = audio.frame_step_ms * options.rate / 1000;

  size_t block_size = 1;
  while (block_size < frame_size)
    block_size *= 2;

  vector<double> window (block_size);
  for (guint i = 0; i < window.size(); i++)
    {
      if (i < frame_size)
        window[i] = window_cos (2.0 * i / frame_size - 1.0);
      else
        window[i] = 0;
    }

  vector<float> sample;

  SineDecoder::Mode mode = options.decoder_mode;

  size_t noise_block_size = NoiseDecoder::preferred_block_size (options.rate);
  NoiseDecoder noise_decoder (options.rate, noise_block_size);
  SineDecoder  sine_decoder (audio.fundamental_freq, options.rate, frame_size, frame_step, mode);

  if (mode != SineDecoder::MODE_TRACKING)
    {
      bool have_phases = true;
      for (auto block : audio.contents)
        {
          if (block.freqs.size() > 0) // only blocks with freqs/mags entries are relevant
            {
              assert (block.freqs.size() == block.mags.size());

              if (block.freqs.size() > block.phases.size())
                {
                  assert (block.phases.size() == 0);
                  have_phases = false;
                }
            }
        }

      if (!have_phases)
        {
          fprintf (stderr, "%s: no phase information found in input data (required by decoder mode)\n", options.program_name.c_str());
          return 1;
        }
    }

  if (options.deterministic_random)
    noise_decoder.set_seed (0x123456);

  size_t pos = 0;
  size_t end_point = audio.contents.size();

  vector<float> decoded_sines (frame_size);

  // decode one frame before actual data (required for tracking decoder)

  if (mode != SineDecoder::MODE_PHASE_SYNC_OVERLAP)
    {
      AudioBlock zero_block;
      const AudioBlock& one_block = audio.contents[0];
      sample.resize (pos + frame_size);

      if (options.sines_enabled)
        {
          sine_decoder.process (zero_block, one_block, window, decoded_sines);
          for (size_t i = 0; i < frame_size; i++)
            sample[pos + i] += decoded_sines[i];
        }
      pos += frame_step;
    }

  // decode sine part of the data
  for (size_t n = 0; n < end_point; n++)
    {
      sample.resize (pos + frame_size);

      if (options.sines_enabled && n + 1 < end_point)
        {
          const AudioBlock& block = audio.contents[n];
          const AudioBlock& next_block = audio.contents[n + 1];

          sine_decoder.process (block, next_block, window, decoded_sines);
          for (size_t i = 0; i < frame_size; i++)
            sample[pos + i] += decoded_sines[i];
        }
      pos += frame_step;
    }

  // decode noise part of the data
  vector<float> decoded_residue (noise_block_size);

  size_t idx = 0;
  for (pos = 0; pos < sample.size() - noise_block_size; pos += noise_block_size / 2)
    {
      /* since the noise block size is not the frame size, we need to find the
       * frame with the center that is closest to the center of the noise block
       * we're about to synthesize
       *
       * in theory, interpolating the noise of the two adjecant frames would
       * give us a better result, but we don't do this for two reasons
       *  - we assume that the noise block size is small (compared to frame size)
       *  - the real audio the user will use is from LiveDecoder anyway
       */
      int noise_center = pos + noise_block_size / 2;
      int frame_center = idx * frame_step + frame_size / 2;
      int next_frame_center = (idx + 1) * frame_step + frame_size / 2;

      while (abs (next_frame_center - noise_center) < abs (frame_center - noise_center))
        {
          idx++;

          frame_center = idx * frame_step + frame_size / 2;
          next_frame_center = (idx + 1) * frame_step + frame_size / 2;
        }

      if (options.noise_enabled && idx < end_point)
        {
          noise_decoder.process (audio.contents[idx].noise.data(), &decoded_residue[0]);
          for (size_t i = 0; i < noise_block_size; i++)
            sample[pos + i] += decoded_residue[i];
        }
    }

  // decode envelope
  for (size_t i = 0; i < sample.size(); i++)
    {
      const double time_ms = i * 1000.0 / options.rate;
      double env;
      if (time_ms < audio.attack_start_ms)
        {
          env = 0;
        }
      else if (time_ms < audio.attack_end_ms)
        {
          env = (time_ms - audio.attack_start_ms) / (audio.attack_end_ms - audio.attack_start_ms);
        }
      else
        {
          env = 1;
        }
      sample[i] *= env * options.gain;
    }

  // strip zero values at start:
  unsigned int zero_values_at_start = audio.zero_values_at_start;
  zero_values_at_start *= double (options.rate) / audio.mix_freq;
  std::copy (sample.begin() + zero_values_at_start, sample.end(), sample.begin());
  sample.resize (sample.size() - zero_values_at_start);

  if (options.export_wav == "")     /* no export -> play */
    {
#if SPECTMORPH_HAVE_AO
      bool clip_err = false;
      pos = 0;
      while (pos < sample.size())
        {
          short svalues[1024];

          int todo = std::min (sample.size() - pos, size_t (1024));
          for (int i = 0; i < todo; i++)
            {
              float f = sample[pos + i];
              float cf = CLAMP (f, -1.0, 1.0);

              if (cf != f && !clip_err)
                {
                  fprintf (stderr, "out of range\n");
                  clip_err = true;
                }

              svalues[i] = cf * 32760;
            }
          //fwrite (svalues, 2, todo, stdout);
          ao_play (play_device, (char *)svalues, 2 * todo);

          pos += todo;
        }
      ao_close (play_device);
#else
      fprintf (stderr, "error: %s was compiled without libao support (use --export <filename>)\n", options.program_name.c_str());
      exit (1);
#endif
    }
  else /* export wav */
    {
      for (size_t i = 0; i < sample.size(); i++)
        {
          float f = sample[i];
          float cf = CLAMP (f, -1.0, 1.0);
          if (f != cf)
            {
              fprintf (stderr, "out of range\n");
              break;
            }
        }

      WavData wav_data (sample, 1, options.rate, options.bits);
      if (!wav_data.save (options.export_wav))
        {
          fprintf (stderr, "%s: export to file %s failed: %s\n", options.program_name.c_str(), options.export_wav.c_str(), wav_data.error_blurb());
          exit(1);
        }
    }
}
