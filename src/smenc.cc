// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <unistd.h>
#include <assert.h>
#include <complex>

#include "smaudio.hh"
#include "smencoder.hh"
#include "smmain.hh"
#include "smdebug.hh"
#include "smutils.hh"
#include "smfft.hh"

#include "config.h"

using std::string;
using std::vector;
using std::min;
using std::max;

using namespace SpectMorph;

static float
freqFromNote (float note)
{
  return 440 * exp (log (2) * (note - 69) / 12.0);
}

/*
 * for ch == 'X', substitute searches for occurrences of %X
 * in pattern and replaces them with the string subst
 */
static void
substitute (string& pattern,
            char    ch,
            const   string& subst)
{
  string result;
  bool need_subst = false;

  for (size_t i = 0; i < pattern.size(); i++)
    {
      if (need_subst)
        {
          if (pattern[i] == ch)
            result += subst;
          else
            {
              result += '%';
              result += pattern[i];
            }
          need_subst = false;
        }
      else if (pattern[i] == '%')
        need_subst = true;
      else
        result += pattern[i];
    }
  if (need_subst)
    pattern += '%';
  pattern = result;
}

/// @cond
struct Options
{
  string	program_name; /* FIXME: what to do with that */
  bool          strip_models;
  bool          text_input_file;
  int           text_input_rate;
  bool          keep_samples;
  bool          attack;
  bool          track_sines;
  float         fundamental_freq;
  int           optimization_level;
  double        loop_start;
  double        loop_end;
  Audio::LoopType loop_type;
  bool          loop_unit_seconds;
  string        debug_decode_filename;
  string        config_filename;

  Options ();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage ();
} options;
/// @endcond

#include "stwutils.hh"

Options::Options ()
{
  program_name = "smenc";
  fundamental_freq = 0; // unset
  optimization_level = 0;
  strip_models = false;
  keep_samples = false;
  track_sines = true;   // perform peak tracking to find sine components
  attack = true;        // perform attack time optimization
  text_input_file = false;
  text_input_rate = 0;
  loop_start = -1;
  loop_end = -1;
  loop_type = Audio::LOOP_NONE;
  loop_unit_seconds = false;
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
	  sm_printf ("%s %s\n", program_name.c_str(), VERSION);
	  exit (0);
	}
      else if (check_arg (argc, argv, &i, "-d"))
	{
          Debug::enable ("encoder");
	}
      else if (check_arg (argc, argv, &i, "-f", &opt_arg))
	{
	  fundamental_freq = sm_atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "-m", &opt_arg))
        {
          fundamental_freq = freqFromNote (atoi (opt_arg));
        }
      else if (check_arg (argc, argv, &i, "-O0"))
        {
          optimization_level = 0;
        }
      else if (check_arg (argc, argv, &i, "-O1"))
        {
          optimization_level = 1;
        }
      else if (check_arg (argc, argv, &i, "-O2"))
        {
          optimization_level = 2;
        }
      else if (check_arg (argc, argv, &i, "-O", &opt_arg))
        {
          optimization_level = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "-s"))
        {
          strip_models = true;
        }
      else if (check_arg (argc, argv, &i, "--keep-samples"))
        {
          keep_samples = true;
        }
      else if (check_arg (argc, argv, &i, "--no-attack"))
        {
          attack = false;
        }
      else if (check_arg (argc, argv, &i, "--no-sines"))
        {
          track_sines = false;
        }
      else if (check_arg (argc, argv, &i, "--debug-decode", &opt_arg))
        {
          debug_decode_filename = opt_arg;
        }
      else if (check_arg (argc, argv, &i, "--loop-start", &opt_arg))
        {
          loop_start = sm_atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--loop-end", &opt_arg))
        {
          loop_end = sm_atof (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--loop-type", &opt_arg))
        {
          if (!Audio::string_to_loop_type (opt_arg, loop_type))
            {
              fprintf (stderr, "%s: unsupported loop type %s\n", options.program_name.c_str(), opt_arg);
              exit (1);
            }
        }
      else if (check_arg (argc, argv, &i, "--loop-unit", &opt_arg))
        {
          if (strcmp (opt_arg, "seconds") != 0)
            {
              fprintf (stderr, "%s: unsupported loop unit %s\n", options.program_name.c_str(), opt_arg);
              exit (1);
            }
          loop_unit_seconds = true;
        }
      else if (check_arg (argc, argv, &i, "--text-input-file", &opt_arg))
        {
          text_input_file = true;
          text_input_rate = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--config", &opt_arg))
        {
          config_filename = opt_arg;
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
  sm_printf ("usage: %s [ <options> ] <src_audio_file> [ <dest_sm_file> ]\n", options.program_name.c_str());
  sm_printf ("\n");
  sm_printf ("options:\n");
  sm_printf (" -h, --help                  help for %s\n", options.program_name.c_str());
  sm_printf (" --version                   print version\n");
  sm_printf (" -f <freq>                   specify fundamental frequency in Hz\n");
  sm_printf (" -m <note>                   specify midi note for fundamental frequency\n");
  sm_printf (" -O <level>                  set optimization level\n");
  sm_printf (" -s                          produced stripped models\n");
  sm_printf (" --no-attack                 skip attack time optimization\n");
  sm_printf (" --no-sines                  skip partial tracking\n");
  sm_printf (" --loop-start                set timeloop start\n");
  sm_printf (" --loop-end                  set timeloop end\n");
  sm_printf (" --debug-decode              debug decode sm file using unquantized values\n");
  sm_printf (" -d                          dump encoder debug information\n");
  sm_printf (" --text-input-file <rate>    set input file format to human readable text values\n");
  sm_printf (" --config <config>           set additional parameters for analysis\n");
  sm_printf ("\n");
}

void
wintrans (const vector<float>& window)
{
  vector<double> in (window.begin(), window.end());
  vector<double> out;

  in.resize (in.size() * 4);
  out.resize (in.size());

  double amp = 0;
  for (size_t i = 0; i < in.size(); i++)
    amp += in[i];

  for (size_t i = 0; i < in.size(); i++)
    in[i] /= amp;

  float *fft_in = FFT::new_array_float (in.size());
  float *fft_out = FFT::new_array_float (in.size());

  std::copy (in.begin(), in.end(), fft_in);
  FFT::fftar_float (in.size(), fft_in, fft_out);
  std::copy (fft_out, fft_out + in.size(), out.begin());

  FFT::free_array_float (fft_out);
  FFT::free_array_float (fft_in);

  for (size_t i = 0; i < out.size(); i += 2)
    {
      double re = out[i];
      double im = out[i + 1];
      double mag = sqrt (re * re + im * im);
      sm_printf ("%zd %g\n", i / 2, mag);
    }
}

size_t
make_odd (size_t n)
{
  if (n & 1)
    return n;
  return n - 1;
}

vector<float>
parse_text_input_file (const string& filename)
{
  char buffer[1024];
  vector<float> signal;

  FILE *input_file = fopen (filename.c_str(), "r");
  if (!input_file)
    {
      fprintf (stderr, "%s: can't open the input file %s\n", options.program_name.c_str(), filename.c_str());
      exit (1);
    }

  int line = 1;
  while (fgets (buffer, 1024, input_file) != NULL)
    {
      if (buffer[0] == '#')
        {
          // skip comments
        }
      else
        {
          char *end_ptr;
          bool parse_error = false;

          signal.push_back (g_ascii_strtod (buffer, &end_ptr));

          // chech that we parsed at least one character
          parse_error = parse_error || (end_ptr == buffer);

          // check that we parsed the whole line
          while (*end_ptr == ' ' || *end_ptr == '\n' || *end_ptr == '\t' || *end_ptr == '\r')
            end_ptr++;
          parse_error = parse_error || (*end_ptr != 0);

          if (parse_error)
            {
              g_printerr ("%s: parse error on line %d\n", options.program_name.c_str(), line);
              exit (1);
            }
        }
      line++;
    }

  fclose (input_file);
  return signal;
}

int
main (int argc, char **argv)
{
  EncoderParams enc_params;

  /* init */
  sm_init (&argc, &argv);
  options.parse (&argc, &argv);

  if (argc != 2 && argc != 3)
    {
      options.print_usage();
      exit (1);
    }

  if (options.config_filename != "")
    {
      if (!enc_params.load_config (options.config_filename))
        {
          fprintf (stderr, "%s: can't open config file '%s'\n", options.program_name.c_str(), options.config_filename.c_str());
          exit (1);
        }
    }

  /* open input */
  string input_file = argv[1];

  WavData wav_data;

  if (options.text_input_file)
    {
      vector<float> text_input_signal = parse_text_input_file (input_file);

      wav_data.load (text_input_signal, 1, options.text_input_rate, 32);
    }
  else
    {
      if (!wav_data.load (input_file))
        {
          fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), input_file.c_str(), wav_data.error_blurb());
          exit (1);
        }
    }

  const double mix_freq = wav_data.mix_freq();
  const int    zeropad  = 4;

  enc_params.mix_freq = mix_freq;
  enc_params.zeropad  = zeropad;
  if (options.fundamental_freq == 0)
    {
      fprintf (stderr, "%s: fundamental freq is required for encoding (use -f or -m options)\n", options.program_name.c_str());
      exit (1);
    }
  enc_params.fundamental_freq = options.fundamental_freq;

  double min_frame_periods, min_frame_size;
  if (!enc_params.get_param ("min-frame-periods", min_frame_periods))
    min_frame_periods = 4;  // default: at least 4 periods of the fundamental per frame
  if (!enc_params.get_param ("min-frame-size", min_frame_size))
    min_frame_size = 40;    // default: at least 40ms frames

  enc_params.frame_size_ms = min_frame_size;
  enc_params.frame_size_ms = max<float> (enc_params.frame_size_ms, 1000 / options.fundamental_freq * min_frame_periods);
  enc_params.frame_step_ms = enc_params.frame_size_ms / 4.0;

  const size_t  frame_size = make_odd (mix_freq * 0.001 * enc_params.frame_size_ms);
  const size_t  frame_step = mix_freq * 0.001 * enc_params.frame_step_ms;

  /* compute block size from frame size (smallest 2^k value >= frame_size) */
  uint64 block_size = 1;
  while (block_size < frame_size)
    block_size *= 2;

  enc_params.frame_step = frame_step;
  enc_params.frame_size = frame_size;
  enc_params.block_size = block_size;

  int n_channels = wav_data.n_channels();

  for (int channel = 0; channel < n_channels; channel++)
    {
      string sm_file;
      if (argc == 2)
        {

          // replace suffix: foo.wav => foo.sm   (or foo-ch1.sm for channel 1)
          size_t dot_pos = input_file.rfind ('.');
          if (dot_pos == string::npos)
            sm_file = input_file;
          else
            sm_file = input_file.substr (0, dot_pos);

          if (n_channels != 1)
            sm_file += string_printf ("-ch%d", channel);
          sm_file += ".sm";
        }
      else if (argc == 3)
        {
          input_file = argv[1];
          sm_file = argv[2];
          substitute (sm_file, 'c', string_printf ("%d", channel));
          if (sm_file == argv[2] && n_channels > 1)
            {
              fprintf (stderr, "%s: input file '%s' has more than one channel, need pattern %%c in output file name.\n", options.program_name.c_str(), input_file.c_str());
              exit (1);
            }
        }

      /* compute encoder window */
      vector<float> window (block_size);

      string window_type;
      if (!enc_params.get_param ("window", window_type))
        window_type = "hann";

      for (guint i = 0; i < window.size(); i++)
        {
          if (i < frame_size)
            {
              if (window_type == "hann")
                {
                  window[i] = window_cos (2.0 * i / (frame_size - 1) - 1.0);
                }
              else if (window_type == "hamming")
                {
                  /* probably never a good idea, since the sidelobes of the spectrum
                   * do not roll off fast (as with the hann window)
                   */
                  window[i] = window_hamming (2.0 * i / (frame_size - 1) - 1.0);
                }
              else if (window_type == "blackman")
                {
                  window[i] = window_blackman (2.0 * i / (frame_size - 1) - 1.0);
                }
              else
                {
                  fprintf (stderr, "%s: unsupported window type in config.\n", options.program_name.c_str());
                  exit (1);
                }
            }
          else
            window[i] = 0;
        }
      enc_params.window = window;

      Encoder encoder (enc_params);
      encoder.encode (wav_data, channel, options.optimization_level, options.attack, options.track_sines);
      if (options.strip_models)
        {
          vector<EncoderBlock>& audio_blocks = encoder.audio_blocks;

          for (size_t i = 0; i < audio_blocks.size(); i++)
            {
              audio_blocks[i].debug_samples.clear();
              audio_blocks[i].original_fft.clear();
            }
          if (!options.keep_samples)
            {
              encoder.original_samples.clear();
            }
        }
      if (options.loop_type == Audio::LOOP_NONE && options.loop_start == -1 && options.loop_end == -1)
        {
          // no loop
        }
      else
        {
          assert (options.loop_type != Audio::LOOP_NONE);
          assert (options.loop_start >= 0 && options.loop_end >= options.loop_start);

          if (options.loop_unit_seconds)
            {
              encoder.set_loop_seconds (options.loop_type, options.loop_start, options.loop_end);
            }
          else
            {
              encoder.set_loop (options.loop_type, options.loop_start, options.loop_end);
            }
        }
      if (options.debug_decode_filename != "")
        encoder.debug_decode (options.debug_decode_filename);

      encoder.save (sm_file);
    }
}
