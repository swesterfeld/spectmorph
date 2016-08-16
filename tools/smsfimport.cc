// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include "config.h"
#include "smgenericin.hh"
#include "smmain.hh"
#include "smjobqueue.hh"
#include "smwavset.hh"
#include "smutils.hh"
#include <stdlib.h>
#include <bse/gsldatahandle.hh>
#include <bse/gsldatautils.hh>

#if 1
static inline void
debug (const char *fmt, ...)
{
}
#else
#define debug printf
#endif

using namespace SpectMorph;

using std::string;
using std::vector;
using std::map;
using std::sort;
using std::set;

enum {
  GEN_PAN            = 17,
  GEN_INSTRUMENT     = 41,
  GEN_KEY_RANGE      = 43,
  GEN_VELOCITY_RANGE = 44,
  GEN_SAMPLE         = 53,
  GEN_SAMPLE_MODES   = 54,
  GEN_ROOT_KEY       = 58
};

struct Generator
{
  int generator;

  // amount generators:
  int amount;

  // range generators:
  int range_min;
  int range_max;

  void read (GenericIn *in);
};

struct Zone
{
  vector<Generator> generators;
};

struct Preset
{
  string       name;
  int          preset;
  int          bank;
  int          preset_bag_index;

  vector<Zone> zones;
};

struct BagEntry
{
  int gen_index;
  int mod_index;
};

struct Instrument
{
  string       name;
  int          instrument_bag_index;
  vector<Zone> zones;
};

struct Sample
{
  string name;
  int    start, end;
  int    startloop, endloop;
  int    srate;
  int    origpitch;
  int    pitchcorrect;
  int    samplelink;
  int    sampletype;
};

vector<Preset>      presets;
vector<BagEntry>    preset_bag;
vector<Generator>   preset_gen;
vector<Instrument>  instruments;
vector<BagEntry>    instrument_bag;
vector<Generator>   instrument_gen;
vector<Sample>      samples;

string
read_fourcc (GenericIn *in)
{
  string fcc;

  for (int i = 0; i < 4; i++)
    {
      char c = in->get_byte();
      fcc += c;
    }

  return fcc;
}

int
read_ui32 (GenericIn *in)
{
  int c0 = in->get_byte();
  int c1 = in->get_byte();
  int c2 = in->get_byte();
  int c3 = in->get_byte();

  return c0 + (c1 << 8) + (c2 << 16) + (c3 << 24);
}

int
read_ui16 (GenericIn *in)
{
  int c0 = in->get_byte();
  int c1 = in->get_byte();

  return c0 + (c1 << 8);
}

int
read_si16 (GenericIn *in)
{
  int c0 = in->get_byte();
  int c1 = int (char (in->get_byte()));

  return c0 + (c1 << 8);
}

string
read_string (GenericIn *in, int len)
{
  string result;

  while (len)
    {
      int b = in->get_byte();
      if (b == 0)
        assert (len == 1 || len == 2);
      else
        result += char (b);
      len--;
    }
  return result;
}

string
read_fixed_string (GenericIn *in, int len)
{
  string result;
  bool   eos = false;
  while (len)
    {
      int b = in->get_byte();

      if (b == 0)
        eos = true;

      if (!eos)
        result += char (b);
      len--;
    }
  return result;
}

void
Generator::read (GenericIn *in)
{
  generator = read_ui16 (in);
  range_min = range_max = amount = 0;
  if (generator == GEN_KEY_RANGE || generator == GEN_VELOCITY_RANGE)
    {
      range_min = in->get_byte();
      range_max = in->get_byte();
    }
  else if (generator == GEN_PAN)
    {
      amount    = read_si16 (in);
    }
  else
    {
      amount    = read_ui16 (in);
    }
}

void
read_ifil (GenericIn *in, int len)
{
  assert (len == 4);
  int major = read_ui16 (in);
  int minor = read_ui16 (in);
  debug ("format version %d.%d\n", major, minor);
}

void
read_INAM (GenericIn *in, int len)
{
  string result = read_string (in, len);
  debug ("name '%s'\n", result.c_str());
}

void
read_isng (GenericIn *in, int len)
{
  string result = read_string (in, len);
  debug ("target '%s'\n", result.c_str());
}

void
read_IPRD (GenericIn *in, int len)
{
  string result = read_string (in, len);
  debug ("product '%s'\n", result.c_str());
}

void
read_IENG (GenericIn *in, int len)
{
  string result = read_string (in, len);
  debug ("engineer '%s'\n", result.c_str());
}

void
read_ISFT (GenericIn *in, int len)
{
  string result = read_string (in, len);
  debug ("tool '%s'\n", result.c_str());
}

void
read_ICRD (GenericIn *in, int len)
{
  string result = read_string (in, len);
  debug ("creation date '%s'\n", result.c_str());
}

void
read_ICMT (GenericIn *in, int len)
{
  string result = read_string (in, len);
  debug ("comment '%s'\n", result.c_str());
}

void
read_ICOP (GenericIn *in, int len)
{
  string result = read_string (in, len);
  debug ("copyright '%s'\n", result.c_str());
}

void
read_phdr (GenericIn *in, int len)
{
  debug ("phdr len = %d\n", len);

  while (len >= 38)
    {
      Preset p;
      p.name = read_fixed_string (in, 20);
      p.preset = read_ui16 (in);
      p.bank = read_ui16 (in);
      p.preset_bag_index = read_ui16 (in);
      presets.push_back (p);

      debug ("preset '%s'\n", p.name.c_str());
      debug ("preset %d\n", p.preset);
      debug ("bank %d\n", p.bank);
      debug ("preset bag ndx %d\n", p.preset_bag_index);
      debug ("library %d\n", read_ui32 (in));
      debug ("genre %d\n", read_ui32 (in));
      debug ("morphology %d\n", read_ui32 (in));
      len -= 38;
    }
}

void
read_pbag (GenericIn *in, int len)
{
  debug ("pbag len = %d\n", len);
  while (len >= 4)
    {
      BagEntry b;
      b.gen_index = read_ui16 (in);
      b.mod_index = read_ui16 (in);
      preset_bag.push_back (b);

      debug ("genndx %d\n", b.gen_index);
      debug ("modndx %d\n", b.mod_index);
      len -= 4;
    }
}

void
read_pmod (GenericIn *in, int len)
{
  debug ("pmod len = %d\n", len);
  while (len >= 10)
    {
      debug ("modsrcoper %d\n", read_ui16 (in));
      debug ("moddestoper %d\n", read_ui16 (in));
      debug ("modamount %d\n", read_ui16 (in));
      debug ("modamtsrcoper %d\n", read_ui16 (in));
      debug ("modtransoper %d\n", read_ui16 (in));
      len -= 10;
    }
}

void
read_pgen (GenericIn *in, int len)
{
  debug ("pgen len = %d\n", len);
  while (len >= 4)
    {
      Generator g;
      g.read (in);
      preset_gen.push_back (g);

      debug ("generator %d\n", g.generator);
      debug ("amount %d\n", g.amount);
      len -= 4;
    }
}

void
read_inst (GenericIn *in, int len)
{
  debug ("inst len = %d\n", len);
  while (len >= 22)
    {
      Instrument i;
      i.name = read_fixed_string (in, 20);
      i.instrument_bag_index = read_ui16 (in);
      instruments.push_back (i);

      debug ("instname %s\n", i.name.c_str());
      debug ("bagindex %d\n", i.instrument_bag_index);
      len -= 22;
    }
}

void
read_ibag (GenericIn *in, int len)
{
  debug ("ibag len = %d\n", len);
  while (len >= 4)
    {
      BagEntry b;
      b.gen_index = read_ui16 (in);
      b.mod_index = read_ui16 (in);
      instrument_bag.push_back (b);

      debug ("instgenidx %d\n", b.gen_index);
      debug ("instmodidx %d\n", b.mod_index);
      len -= 4;
    }
}

void
read_imod (GenericIn *in, int len)
{
  debug ("imod len = %d\n", len);
  while (len >= 10)
    {
      debug ("modsrcoper %d\n", read_ui16 (in));
      debug ("moddestoper %d\n", read_ui16 (in));
      debug ("modamount %d\n", read_ui16 (in));
      debug ("modamtsrcoper %d\n", read_ui16 (in));
      debug ("modtransoper %d\n", read_ui16 (in));
      len -= 10;
    }
}

void
read_igen (GenericIn *in, int len)
{
  debug ("igen len = %d\n", len);
  while (len >= 4)
    {
      Generator g;
      g.read (in);
      instrument_gen.push_back (g);

      debug ("generator %d\n", g.generator);
      debug ("amount %d\n", g.amount);
      len -= 4;
    }
}

void
read_shdr (GenericIn *in, int len)
{
  debug ("shdr len = %d\n", len);
  while (len >= 46)
    {
      Sample s;
      s.name = read_fixed_string (in, 20);
      s.start = read_ui32 (in);
      s.end = read_ui32 (in);
      s.startloop = read_ui32 (in);
      s.endloop = read_ui32 (in);
      s.srate = read_ui32 (in);
      s.origpitch = in->get_byte();
      s.pitchcorrect = in->get_byte();
      s.samplelink = read_ui16 (in);
      s.sampletype = read_ui16 (in);
      samples.push_back (s);

      debug ("samplename %s\n", s.name.c_str());
      debug ("start %d\n", s.start);
      debug ("end %d\n", s.end);
      debug ("startloop %d\n", s.startloop);
      debug ("endloop %d\n", s.endloop);
      debug ("srate %d\n", s.srate);
      debug ("origpitch %d\n", s.origpitch);
      debug ("pitchcorrect %d\n", s.pitchcorrect);
      debug ("samplelink %d\n", s.samplelink);
      debug ("sampletype %d\n", s.sampletype);
      len -= 46;
    }
}

const char *
gen2name (int i)
{
  struct G2N
  {
    int         gen;
    const char *name;
  } g2n[] = {
    { 8,                  "initial-filter-fc" },
    { GEN_PAN,            "pan" },
    { 35,                 "vol-env-hold" },
    { 36,                 "vol-env-decay" },
    { 37,                 "vol-env-sustain" },
    { 38,                 "vol-env-release" },
    { GEN_INSTRUMENT,     "instrument" },
    { GEN_KEY_RANGE,      "key-range" },
    { GEN_VELOCITY_RANGE, "velocity-range" },
    { GEN_SAMPLE,         "sample-id" },
    { GEN_SAMPLE_MODES,   "sample-modes" },
    { GEN_ROOT_KEY,       "root-key" },
    { 0, NULL }
  };
  for (int k = 0; g2n[k].name; k++)
    if (g2n[k].gen == i)
      return g2n[k].name;
  return "unknown";
}

const Generator *
find_gen (int id, const vector<Generator>& generators)
{
  for (vector<Generator>::const_iterator gi = generators.begin(); gi != generators.end(); gi++)
    if (gi->generator == id)
      return &(*gi);

  return NULL; // not found
}

void
xsystem (const string& cmd)
{
  printf ("# %s\n", cmd.c_str());
  int rc = system (cmd.c_str());
  if (rc != 0)
    {
      printf ("command execution failed: %d\n", rc);
      exit (1);
    }
}

vector<float> sample_data;

struct Options
{
  string              program_name;
  enum { NONE, LIST, DUMP, IMPORT } command;
  int                 midi_note;
  bool                fast_import;
  bool                debug;
  bool                mono_flat;
  int                 max_jobs;
  string              config_filename;
  string              smenc;
  string              output_filename;

  Options();
  void parse (int *argc_p, char **argv_p[]);
  static void print_usage();
} options;

#include "stwutils.hh"

Options::Options()
{
  command = NONE;
  program_name = "smsfimport";
  midi_note = -1; // all
  fast_import = false;
  debug = false;
  max_jobs = 1;
  smenc = "smenc";
  mono_flat = false;
}

void
Options::parse (int   *argc_p,
                char **argv_p[])
{
  guint argc = *argc_p;
  gchar **argv = *argv_p;
  unsigned int i, e;

  g_return_if_fail (argc >= 0);

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
      else if (check_arg (argc, argv, &i, "-m", &opt_arg))
        {
          midi_note = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "-j", &opt_arg))
        {
          max_jobs = atoi (opt_arg);
        }
      else if (check_arg (argc, argv, &i, "--fast"))
        {
          fast_import = true;
        }
      else if (check_arg (argc, argv, &i, "--debug"))
        {
          debug = true;
        }
      else if (check_arg (argc, argv, &i, "--config", &opt_arg))
        {
          config_filename = opt_arg;
        }
      else if (check_arg (argc, argv, &i, "--cache"))
        {
          smenc = "smenccache";
        }
      else if (check_arg (argc, argv, &i, "--output", &opt_arg))
        {
          output_filename = opt_arg;
        }
      else if (check_arg (argc, argv, &i, "--mono-flat"))
        {
          mono_flat = true;
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
          string str = argv[1];
          if (str == "list")
            {
              command = LIST;
            }
          else if (str == "dump")
            {
              command = DUMP;
            }
          else if (str == "import")
            {
              command = IMPORT;
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
  printf (" %s list [ <options> ] <sf2_filename>\n", options.program_name.c_str());
  printf (" %s dump [ <options> ] <sf2_filename> [ <preset_name> ]\n", options.program_name.c_str());
  printf (" %s import [ <options> ] <sf2_filename> <preset_name>\n", options.program_name.c_str());
  printf ("\n");
  printf ("options:\n");
  printf (" -h, --help                  help for %s\n", options.program_name.c_str());
  printf (" -v, --version               print version\n");
  printf ("\n");
}

int
read_sf2 (const string& filename)
{
  GenericIn *in = GenericIn::open (filename.c_str());
  if (!in)
    {
      fprintf (stderr, "%s: error opening file %s\n", options.program_name.c_str(), filename.c_str());
      return 1;
    }

  string fcc = read_fourcc (in);
  if (fcc != "RIFF")
    {
      fprintf (stderr, "not a RIFF file\n");
      return 1;
    }

  int len = read_ui32 (in);
  debug ("len = %d\n", len);

  fcc = read_fourcc (in);
  if (fcc != "sfbk")
    {
      fprintf (stderr, "missing sfbk chunk\n");
      return 1;
    }
  fcc = read_fourcc (in);
  debug ("fcc<list> = %s\n", fcc.c_str());
  len = read_ui32 (in);
  debug ("len = %d\n", len);

  size_t list_end_pos = len + in->get_pos();

  fcc = read_fourcc (in);
  debug ("fcc<info> = %s\n", fcc.c_str());

  while (in->get_pos() < list_end_pos)
    {
      fcc = read_fourcc (in);
      len = read_ui32 (in);

      if (fcc == "ifil")
        read_ifil (in, len);
      else if (fcc == "INAM")
        read_INAM (in, len);
      else if (fcc == "isng")
        read_isng (in, len);
      else if (fcc == "IPRD")
        read_IPRD (in, len);
      else if (fcc == "IENG")
        read_IENG (in, len);
      else if (fcc == "ISFT")
        read_ISFT (in, len);
      else if (fcc == "ICRD")
        read_ICRD (in, len);
      else if (fcc == "ICMT")
        read_ICMT (in, len);
      else if (fcc == "ICOP")
        read_ICOP (in, len);
      else
        {
          printf ("unhandled chunk: %s\n", fcc.c_str());
          return 1;
        }
    }

  debug ("position = %zd\n", in->get_pos());
  fcc = read_fourcc (in);
  debug ("fcc<list> = %s\n", fcc.c_str());
  len = read_ui32 (in);
  debug ("len = %d\n", len);

  fcc = read_fourcc (in);
  debug ("fcc<info> = %s\n", fcc.c_str());
  fcc = read_fourcc (in);
  debug ("fcc<info> = %s\n", fcc.c_str());
  len = read_ui32 (in);
  debug ("len = %d\n", len);

  while (len)
    {
      sample_data.push_back (read_si16 (in) * (1 / 32768.0));
      len -= 2;
    }
  debug ("sample_data len: %zd\n", sample_data.size());

  fcc = read_fourcc (in);
  debug ("fcc<list> = %s\n", fcc.c_str());
  len = read_ui32 (in);
  debug ("len = %d\n", len);

  list_end_pos = len + in->get_pos();

  fcc = read_fourcc (in);
  debug ("fcc<pdta> = %s\n", fcc.c_str());

  while (in->get_pos() < list_end_pos)
    {
      fcc = read_fourcc (in);
      len = read_ui32 (in);

      if (fcc == "phdr")
        read_phdr (in, len);
      else if (fcc == "pbag")
        read_pbag (in, len);
      else if (fcc == "pmod")
        read_pmod (in, len);
      else if (fcc == "pgen")
        read_pgen (in, len);
      else if (fcc == "inst")
        read_inst (in, len);
      else if (fcc == "ibag")
        read_ibag (in, len);
      else if (fcc == "imod")
        read_imod (in, len);
      else if (fcc == "igen")
        read_igen (in, len);
      else if (fcc == "shdr")
        read_shdr (in, len);
      else
        {
          printf ("unhandled chunk: %s\n", fcc.c_str());
          in->skip (len);
        }
    }
  assert (in->get_byte() == -1);

  for (vector<Preset>::iterator pi = presets.begin(); pi != presets.end(); pi++)
    {
      debug ("Preset %s\n", pi->name.c_str());
      debug ("  bank %d\n", pi->bank);
      debug ("  preset %d\n", pi->preset);

      if ((pi + 1) < presets.end())
        {

          int start = pi->preset_bag_index, end = (pi + 1)->preset_bag_index;
          for (vector<BagEntry>::iterator bi = preset_bag.begin() + start; bi != preset_bag.begin() + end; bi++)
            {
              Zone zone; // FIXME! needs to be in inner loop

              debug ("    genndx %d\n", bi->gen_index);
              debug ("    modndx %d\n", bi->mod_index);
              if ((bi + 1) < preset_bag.end())
                for (int gndx = bi->gen_index; gndx < (bi + 1)->gen_index; gndx++)
                  {
                    debug ("      generator %d (%s)\n", preset_gen[gndx].generator, gen2name (preset_gen[gndx].generator));
                    debug ("      amount %d\n", preset_gen[gndx].amount);
                    if (preset_gen[gndx].generator == 41) // instrument id
                      {
                        size_t id = preset_gen[gndx].amount;
                        assert (id >= 0 && id < instruments.size());
                        debug ("        -> %s\n", instruments[id].name.c_str());
                      }
                    zone.generators.push_back (preset_gen[gndx]);
                  }
              pi->zones.push_back (zone);
            }
        }
    }
  for (vector<Instrument>::iterator ii = instruments.begin(); ii != instruments.end(); ii++)
    {
      debug ("Instrument %s\n", ii->name.c_str());
      if ((ii + 1) < instruments.end())
        {

          int start = ii->instrument_bag_index, end = (ii + 1)->instrument_bag_index;
          for (vector<BagEntry>::iterator bi = instrument_bag.begin() + start; bi != instrument_bag.begin() + end; bi++)
            {
              Zone zone;

              debug ("    genndx %d\n", bi->gen_index);
              debug ("    modndx %d\n", bi->mod_index);
              if ((bi + 1) < instrument_bag.end())
                for (int gndx = bi->gen_index; gndx < (bi + 1)->gen_index; gndx++)
                  {
                    debug ("      generator %d (%s)\n", instrument_gen[gndx].generator, gen2name (instrument_gen[gndx].generator));
                    debug ("      amount %d\n", instrument_gen[gndx].amount);
                    if (instrument_gen[gndx].generator == 53) // sample id
                      {
                        size_t id = instrument_gen[gndx].amount;
                        assert (id >= 0 && id < samples.size());
                        debug ("        -> %s\n", samples[id].name.c_str());
                      }
                    zone.generators.push_back (instrument_gen[gndx]);
                  }
              ii->zones.push_back (zone);
            }
        }
    }
#if 0
  for (vector<Sample>::iterator si = samples.begin(); si != samples.end(); si++)
    {
      printf ("Sample %s\n", si->name.c_str());
    }
#endif
  delete in;
  return 0;
}

string
check_import (const Preset& p)
{
  if (p.zones.size() == 1)
    {
      //printf ("PRESET: %s\n", p.name.c_str());
      const Zone& zone = p.zones[0];
      int inst_index = -1;
      for (vector<Generator>::const_iterator gi = zone.generators.begin(); gi != zone.generators.end(); gi++)
        {
          if (gi->generator == GEN_INSTRUMENT)
            {
              assert (inst_index == -1);
              inst_index = gi->amount;
            }
        }
      assert (inst_index >= 0);
      //printf ("inst_index=%d\n", inst_index);

      vector<int> key_count (128);

      const Instrument& instrument = instruments[inst_index];
      for (vector<Zone>::const_iterator zi = instrument.zones.begin(); zi != instrument.zones.end(); zi++)
        {
          // const size_t zone_index = zi - instrument.zones.begin();
          // printf ("zone %zd:\n", zone_index);
          // int root_key = -1;

          const Generator *gi; // = find_gen (GEN_ROOT_KEY, zi->generators);
          //if (gi)
          //root_key = gi->amount;

          gi = find_gen (GEN_SAMPLE, zi->generators);
          if (gi)
            {
              size_t id = gi->amount;
              assert (id >= 0 && id < samples.size());
              //printf (" * sample = %s\n", samples[id].name.c_str());

              /* default: map to all keys */
              int kr_min = 0;
              int kr_max = 127;

              gi = find_gen (GEN_KEY_RANGE, zi->generators);
              if (gi)
                {
                  kr_min = gi->range_min;
                  kr_max = gi->range_max;
                }
              //printf (" * key range = %d..%d\n", kr_min, kr_max);
              for (int i = kr_min; i < kr_max; i++)
                key_count[i] += 1;
            }
        }
      for (size_t i = 0; i < key_count.size(); i++)
        if (key_count[i] > 1)
          return "noimport (key range overlap)";

      return "importable";
    }
  else
    return "noimport (> 1 zone)";
}

void
list_sf2()
{
  vector<string> preset_out;
  for (vector<Preset>::iterator pi = presets.begin(); pi < presets.end() - 1; pi++)
    preset_out.push_back (string_printf ("%03d:%03d %s", pi->bank, pi->preset, pi->name.c_str()));

  // FIXME: check_import (*pi).c_str()

  sort (preset_out.begin(), preset_out.end());
  for (vector<string>::iterator poi = preset_out.begin(); poi != preset_out.end(); poi++)
    printf ("%s\n", poi->c_str());
}

void
run_all (vector<string>& commands, const string& name, size_t max_jobs)
{
  printf ("Running %s commands...\n", name.c_str());
  JobQueue job_queue (max_jobs);
  for (vector<string>::iterator ci = commands.begin(); ci != commands.end(); ci++)
    {
      printf (" - %s\n", ci->c_str());
      job_queue.run (*ci);
    }
  if (!job_queue.wait_for_all())
    {
      fprintf (stderr, "error executing %s commands\n", name.c_str());
    }
}

void
make_mono_flat (WavSet& wav_set)
{
  vector<WavSetWave> flat_waves;

  map<int, string> wmap;

  for (auto wave: wav_set.waves)
    {
      if (wave.channel == 0)
        {
          string& path = wmap[wave.midi_note];
          if (path.empty())
            {
              path = wave.path;

              WavSetWave new_wave;
              new_wave.midi_note = wave.midi_note;
              new_wave.path = wave.path;
              new_wave.channel = wave.channel;
              new_wave.velocity_range_min = 0;
              new_wave.velocity_range_max = 127;

              flat_waves.push_back (new_wave);
            }
          else
            assert (path == wave.path);
        }
    }
  wav_set.waves = flat_waves;
}

int
import_preset (const string& import_name)
{
  map<string,bool> is_encoded;

  for (vector<Preset>::iterator pi = presets.begin(); pi != presets.end(); pi++)
    {
      if (pi->name == import_name)
        {
          printf ("importing preset %s\n", import_name.c_str());

          string output_filename = options.output_filename;
          if (output_filename == "")
            {
              string preset_xname;
              for (string::const_iterator ni = import_name.begin(); ni != import_name.end(); ni++)
                {
                  char c = *ni;
                  if (isupper (c))
                    c = tolower (c);
                  else if (islower (c))
                    ;
                  else if (isdigit (c))
                    ;
                  else
                    c = '_';
                  preset_xname += c;
                }
              printf ("%s\n", preset_xname.c_str());
              output_filename = preset_xname + ".smset";
            }

          WavSet wav_set;

          vector<string> enc_commands, strip_commands;
          for (vector<Zone>::iterator preset_zi = pi->zones.begin(); preset_zi != pi->zones.end(); preset_zi++)
            {
              Zone& zone = *preset_zi;
              int inst_index = -1;
              for (vector<Generator>::iterator gi = zone.generators.begin(); gi != zone.generators.end(); gi++)
                {
                  if (gi->generator == GEN_INSTRUMENT)
                    {
                      assert (inst_index == -1);
                      inst_index = gi->amount;
                    }
                }
              int vr_min = 0, vr_max = 127;
              const Generator *gp = find_gen (GEN_VELOCITY_RANGE, zone.generators);
              if (gp)
                {
                  vr_min = gp->range_min;
                  vr_max = gp->range_max;
                }
              printf ("  * velocity range: %d..%d\n", vr_min, vr_max);
              // FIXME: this is a little sloppy - we should accept inst_index < 0 only for global zone
              if (inst_index >= 0)
                {
                  Instrument instrument = instruments[inst_index];
                  printf ("instrument name: %s (%zd zones)\n", instrument.name.c_str(), instrument.zones.size());

                  for (vector<Zone>::iterator zi = instrument.zones.begin(); zi != instrument.zones.end(); zi++)
                    {
                      const size_t zone_index = zi - instrument.zones.begin();

                      printf ("zone %zd:\n", zone_index);

                      int root_key = -1;
                      const Generator *gi = find_gen (GEN_ROOT_KEY, zi->generators);
                      if (gi)
                        root_key = gi->amount;

                      int sample_modes = 0;
                      gi = find_gen (GEN_SAMPLE_MODES, zi->generators);
                      if (gi)
                        sample_modes = gi->amount;

                      int channel = 0;
                      gi = find_gen (GEN_PAN, zi->generators);
                      if (gi)
                        {
                          assert (gi->amount == -500 || gi->amount == 500);

                          if (gi->amount == -500)
                            channel = 0;
                          else if (gi->amount == 500)
                            channel = 1;
                          else
                            assert (false);
                        }

                      gi = find_gen (GEN_SAMPLE, zi->generators);
                      if (gi)
                        {
                          size_t id = gi->amount;
                          assert (id >= 0 && id < samples.size());
                          int midi_note = (root_key >= 0) ? root_key : samples[id].origpitch;

                          if (options.midi_note == -1 || (midi_note == options.midi_note))
                            {
                              printf (" sample %s orig_pitch %d root_key %d => midi_note %d\n", samples[id].name.c_str(), samples[id].origpitch, root_key, midi_note);

                              string filename = string_printf ("sample%zd-%d.wav", id, midi_note);
                              string smname = string_printf ("sample%zd-%d.sm", id, midi_note);

                              if (!is_encoded[smname])
                                {
                                  vector<float> padded_sample;
                                  size_t padded_len;
                                  size_t loop_shift = 0.1 * samples[id].srate;   // 100 ms loop shift
                                  string loop_args;
                                  if (sample_modes & 1)
                                    {
                                      loop_args += " --loop-type loop-time-forward";
                                      loop_args += string_printf (" --loop-start %zd",
                                                                  samples[id].startloop - samples[id].start +
                                                                  loop_shift);
                                      loop_args += string_printf (" --loop-end %zd",
                                                                  samples[id].endloop - samples[id].start +
                                                                  loop_shift);

                                      // 200 ms padding at the end of the loop, to ensure that silence after sample
                                      // is not encoded by encoder
                                      padded_len = samples[id].end - samples[id].start + 0.2 * samples[id].srate;
                                      for (size_t i = 0; i < padded_len; i++)
                                        {
                                          size_t pos = i + samples[id].start;
                                          while (pos >= (size_t) samples[id].endloop)
                                            pos -= samples[id].endloop - samples[id].startloop;
                                          padded_sample.push_back (sample_data[pos]);
                                        }
                                    }
                                  else
                                    {
                                      // no padding
                                      padded_len = samples[id].end - samples[id].start;
                                      padded_sample.assign (&sample_data[samples[id].start], &sample_data[samples[id].end]);
                                    }
                                  GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, samples[id].srate, 440,
                                                                                    padded_len, &padded_sample[0], NULL);
                                  gsl_data_handle_open (dhandle);

                                  int fd = open (filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                                  if (fd < 0)
                                    {
                                      Bse::Error error = bse_error_from_errno (errno, Bse::Error::FILE_OPEN_FAILED);
                                      sfi_error ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
                                    }

                                  int xerrno = gsl_data_handle_dump_wav (dhandle, fd, 16, dhandle->setup.n_channels, (guint) dhandle->setup.mix_freq);
                                  if (xerrno)
                                    {
                                      Bse::Error error = bse_error_from_errno (xerrno, Bse::Error::FILE_WRITE_FAILED);
                                      sfi_error ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
                                    }
                                  close (fd);

                                  string import_args = options.fast_import ? "--no-attack -O0" : "-O1";
                                  if (options.config_filename != "")
                                    import_args += " --config " + options.config_filename;

                                  enc_commands.push_back (
                                    string_printf ("%s -m %d %s %s %s %s", options.smenc.c_str(),
                                                   midi_note, import_args.c_str(),
                                                   filename.c_str(), smname.c_str(), loop_args.c_str()));
                                  if (!options.debug)
                                    strip_commands.push_back (
                                      string_printf ("smstrip --keep-samples %s", smname.c_str()));

                                  is_encoded[smname] = true;
                                }
                              WavSetWave new_wave;
                              new_wave.midi_note = midi_note;
                              new_wave.path = smname;
                              new_wave.channel = channel;
                              new_wave.velocity_range_min = vr_min;
                              new_wave.velocity_range_max = vr_max;

                              wav_set.waves.push_back (new_wave);
                            }
                        }
                    }
                }
            }
          run_all (enc_commands, "Encoder", options.max_jobs);
          run_all (strip_commands, "Strip", options.max_jobs);

          if (options.mono_flat)
            make_mono_flat (wav_set);

          wav_set.save (output_filename);
          xsystem (string_printf ("smwavset link %s", output_filename.c_str()));
        }
    }
  return 0;
}

void
dump (const string& preset_name = "")
{
  set<int> dump_instr;
  set<int> dump_sample;

  for (vector<Preset>::iterator pi = presets.begin(); pi != presets.end(); pi++)
    {
      if (pi->name == preset_name || preset_name.empty())
        {
          printf ("PRESET %s\n", pi->name.c_str());
          for (vector<Zone>::iterator zi = pi->zones.begin(); zi != pi->zones.end(); zi++)
            {
              const size_t zone_index = zi - pi->zones.begin();
              printf ("  Zone #%zd\n", zone_index);
              for (vector<Generator>::const_iterator gi = zi->generators.begin();
                                                     gi != zi->generators.end(); gi++)
                {
                  const size_t generator_index = gi - zi->generators.begin();
                  printf ("    Generator #%zd: %d (%s)\n", generator_index, gi->generator, gen2name (gi->generator));
                  if (gi->generator == GEN_INSTRUMENT)
                    dump_instr.insert (gi->amount);
                }
            }
        }
    }
  for (vector<Instrument>::iterator ii = instruments.begin(); ii != instruments.end(); ii++)
    {
      size_t inr = ii - instruments.begin();
      if (dump_instr.count (inr))
        {
          printf ("INSTRUMENT %s\n", ii->name.c_str());
          for (vector<Zone>::iterator zi = ii->zones.begin(); zi != ii->zones.end(); zi++)
            {
              const size_t zone_index = zi - ii->zones.begin();
              printf ("  Zone #%zd\n", zone_index);
              for (vector<Generator>::const_iterator gi = zi->generators.begin();
                                                     gi != zi->generators.end(); gi++)
                {
                  const size_t generator_index = gi - zi->generators.begin();
                  printf ("    Generator #%zd: %d (%s)\n", generator_index, gi->generator, gen2name (gi->generator));
                  printf ("                    amount %d\n", gi->amount);

                  if (gi->generator == GEN_SAMPLE)
                    dump_sample.insert (gi->amount);
                }
            }
        }
    }
  for (vector<Sample>::iterator si = samples.begin(); si != samples.end(); si++)
    {
      size_t snr = si - samples.begin();
      if (dump_sample.count (snr))
        {
          printf ("Sample %s:\n", si->name.c_str());
          printf ("  Length: %d\n", si->end - si->start);
          printf ("  Loop  : %d..%d\n", si->startloop - si->start, si->endloop - si->start);
          printf ("  SRate : %d\n", si->srate);
        }
    }
}

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);
  options.parse (&argc, &argv);

  if (options.command == Options::LIST)
    {
      assert (argc == 2);
      if (read_sf2 (argv[1]) != 0)
        {
          printf ("can't load sf2: %s\n", argv[1]);
          return 1;
        }
      list_sf2();
    }
  else if (options.command == Options::IMPORT)
    {
      assert (argc == 3);
      if (read_sf2 (argv[1]) != 0)
        {
          printf ("can't load sf2: %s\n", argv[1]);
          return 1;
        }
      if (import_preset (argv[2]) != 0)
        {
          printf ("can't import preset: %s\n", argv[2]);
          return 1;
        }
    }
  else if (options.command == Options::DUMP)
    {
      assert (argc == 2 || argc == 3);

      if (read_sf2 (argv[1]) != 0)
        {
          printf ("can't load sf2: %s\n", argv[1]);
          return 1;
        }

      if (argc == 2)
        dump();
      else
        dump (argv[2]);
    }
  else
    {
      printf ("You need to specify a command (import, list, dump).\n\n");
      Options::print_usage();
      exit (1);
    }

  return 0;
}
