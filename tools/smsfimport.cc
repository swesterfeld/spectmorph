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

#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <vector>

#include "smgenericin.hh"
#include <stdlib.h>
#include <bse/gsldatahandle.h>
#include <bse/gsldatautils.h>
#include <bse/bsemain.h>

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

enum {
  GEN_INSTRUMENT    = 41,
  GEN_SAMPLE        = 53,
  GEN_SAMPLE_MODES  = 54,
  GEN_ROOT_KEY      = 58
};

struct Generator
{
  int generator;
  int amount;
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
      g.generator = read_ui16 (in);
      g.amount    = read_ui16 (in);
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
      g.generator = read_ui16 (in);
      g.amount    = read_ui16 (in);
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
    { 8,  "initial-filter-fc" },
    { 35, "vol-env-hold" },
    { 36, "vol-env-decay" },
    { 41, "instrument" },
    { 43, "key-range" },
    { 44, "velocity-range" },
    { GEN_SAMPLE,       "sample-id" },
    { GEN_SAMPLE_MODES, "sample-modes" },
    { GEN_ROOT_KEY,     "root-key" },
    { 0, NULL }
  };
  for (int k = 0; g2n[k].name; k++)
    if (g2n[k].gen == i)
      return g2n[k].name;
  return "unknown";
}

Generator *
find_gen (int id, vector<Generator>& generators)
{
  for (vector<Generator>::iterator gi = generators.begin(); gi != generators.end(); gi++)
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

int
main (int argc, char **argv)
{
  /* init */
  SfiInitValue values[] = {
    { "stand-alone",            "true" }, /* no rcfiles etc. */
    { "wave-chunk-padding",     NULL, 1, },
    { "dcache-block-size",      NULL, 8192, },
    { "dcache-cache-memory",    NULL, 5 * 1024 * 1024, },
    { NULL }
  };
  bse_init_inprocess (&argc, &argv, NULL, values);

  assert (argc == 2 || argc == 3);

  GenericIn *in = GenericIn::open (argv[1]);
  if (!in)
    {
      fprintf (stderr, "%s: error opening file %s\n", argv[0], argv[1]);
      return 1;
    }

  string fcc = read_fourcc (in);
  if (fcc != "RIFF")
    {
      fprintf (stderr, "not a RIFF file\n");
      return 1;
    }

  int len = read_ui32 (in);
  printf ("len = %d\n", len);

  fcc = read_fourcc (in);
  if (fcc != "sfbk")
    {
      fprintf (stderr, "missing sfbk chunk\n");
      return 1;
    }
  fcc = read_fourcc (in);
  printf ("fcc<list> = %s\n", fcc.c_str());
  len = read_ui32 (in);
  printf ("len = %d\n", len);

  size_t list_end_pos = len + in->get_pos();

  fcc = read_fourcc (in);
  printf ("fcc<info> = %s\n", fcc.c_str());

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

  printf ("position = %zd\n", in->get_pos());
  fcc = read_fourcc (in);
  printf ("fcc<list> = %s\n", fcc.c_str());
  len = read_ui32 (in);
  printf ("len = %d\n", len);

  fcc = read_fourcc (in);
  printf ("fcc<info> = %s\n", fcc.c_str());
  fcc = read_fourcc (in);
  printf ("fcc<info> = %s\n", fcc.c_str());
  len = read_ui32 (in);
  printf ("len = %d\n", len);

  vector<float> sample_data;
  while (len)
    {
      sample_data.push_back (read_si16 (in) * (1 / 32768.0));
      len -= 2;
    }
  printf ("sample_data len: %zd\n", sample_data.size());

  fcc = read_fourcc (in);
  printf ("fcc<list> = %s\n", fcc.c_str());
  len = read_ui32 (in);
  printf ("len = %d\n", len);

  list_end_pos = len + in->get_pos();

  fcc = read_fourcc (in);
  printf ("fcc<pdta> = %s\n", fcc.c_str());

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
      printf ("Preset %s\n", pi->name.c_str());
      printf ("  bank %d\n", pi->bank);
      printf ("  preset %d\n", pi->preset);

      if ((pi + 1) < presets.end())
        {
          Zone zone; // FIXME! needs to be in inner loop

          int start = pi->preset_bag_index, end = (pi + 1)->preset_bag_index;
          for (vector<BagEntry>::iterator bi = preset_bag.begin() + start; bi != preset_bag.begin() + end; bi++)
            {
              printf ("    genndx %d\n", bi->gen_index);
              printf ("    modndx %d\n", bi->mod_index);
              if ((bi + 1) < preset_bag.end())
                for (int gndx = bi->gen_index; gndx < (bi + 1)->gen_index; gndx++)
                  {
                    printf ("      generator %d (%s)\n", preset_gen[gndx].generator, gen2name (preset_gen[gndx].generator));
                    printf ("      amount %d\n", preset_gen[gndx].amount);
                    if (preset_gen[gndx].generator == 41) // instrument id
                      {
                        size_t id = preset_gen[gndx].amount;
                        assert (id >= 0 && id < instruments.size());
                        printf ("        -> %s\n", instruments[id].name.c_str());
                      }
                    zone.generators.push_back (preset_gen[gndx]);
                  }
            }
          pi->zones.push_back (zone);
        }
    }
  for (vector<Instrument>::iterator ii = instruments.begin(); ii != instruments.end(); ii++)
    {
      printf ("Instrument %s\n", ii->name.c_str());
      if ((ii + 1) < instruments.end())
        {

          int start = ii->instrument_bag_index, end = (ii + 1)->instrument_bag_index;
          for (vector<BagEntry>::iterator bi = instrument_bag.begin() + start; bi != instrument_bag.begin() + end; bi++)
            {
              Zone zone;

              printf ("    genndx %d\n", bi->gen_index);
              printf ("    modndx %d\n", bi->mod_index);
              if ((bi + 1) < instrument_bag.end())
                for (int gndx = bi->gen_index; gndx < (bi + 1)->gen_index; gndx++)
                  {
                    printf ("      generator %d (%s)\n", instrument_gen[gndx].generator, gen2name (instrument_gen[gndx].generator));
                    printf ("      amount %d\n", instrument_gen[gndx].amount);
                    if (instrument_gen[gndx].generator == 53) // sample id
                      {
                        size_t id = instrument_gen[gndx].amount;
                        assert (id >= 0 && id < samples.size());
                        printf ("        -> %s\n", samples[id].name.c_str());
                      }
                    zone.generators.push_back (instrument_gen[gndx]);
                  }
              ii->zones.push_back (zone);
            }
        }
    }
  for (vector<Sample>::iterator si = samples.begin(); si != samples.end(); si++)
    {
      printf ("Sample %s\n", si->name.c_str());
    }
  if (argc == 3)
    {
      for (vector<Preset>::iterator pi = presets.begin(); pi != presets.end(); pi++)
        {
          if (pi->name == argv[2])
            {
              printf ("importing preset %s\n", argv[2]);
              if (pi->zones.size() != 1)
                {
                  printf ("preset has %zd zones (should be 1) - can't import\n", pi->zones.size());
                  return 1;
                }

              string preset_xname;
              for (int i = 0; argv[2][i]; i++)
                {
                  char c = argv[2][i];
                  if (isupper (c))
                    c = tolower (c);
                  else if (islower (c))
                    ;
                  else
                    c = '_';
                  preset_xname += c;
                }
              printf ("%s\n", preset_xname.c_str());

              string cmd = Birnet::string_printf ("smwavset init %s.smset", preset_xname.c_str());
              xsystem (cmd);

              Zone& zone = pi->zones[0];
              int inst_index = -1;
              for (vector<Generator>::iterator gi = zone.generators.begin(); gi != zone.generators.end(); gi++)
                {
                  if (gi->generator == GEN_INSTRUMENT)
                    {
                      assert (inst_index == -1);
                      inst_index = gi->amount;
                    }
                }
              assert (inst_index >= 0);
              Instrument instrument = instruments[inst_index];
              printf ("instrument name: %s (%zd zones)\n", instrument.name.c_str(), instrument.zones.size());

              for (vector<Zone>::iterator zi = instrument.zones.begin(); zi != instrument.zones.end(); zi++)
                {
                  const size_t zone_index = zi - instrument.zones.begin();

                  printf ("zone %zd:\n", zone_index);
                  string filename = Birnet::string_printf ("zone%zd.wav", zone_index);
                  string smname = Birnet::string_printf ("zone%zd.sm", zone_index);

                  int root_key = -1;
                  Generator *gi = find_gen (GEN_ROOT_KEY, zi->generators);
                  if (gi)
                    root_key = gi->amount;

                  int sample_modes = 0;
                  gi = find_gen (GEN_SAMPLE_MODES, zi->generators);
                  if (gi)
                    sample_modes = gi->amount;

                  gi = find_gen (GEN_SAMPLE, zi->generators);
                  if (gi)
                    {
                      size_t id = gi->amount;
                      assert (id >= 0 && id < samples.size());
                      int midi_note = (root_key >= 0) ? root_key : samples[id].origpitch;

                      printf (" sample %s orig_pitch %d root_key %d => midi_note %d\n", samples[id].name.c_str(), samples[id].origpitch, root_key, midi_note);

                      GslDataHandle *dhandle = gsl_data_handle_new_mem (1, 32, samples[id].srate, 440, samples[id].end - samples[id].start, &sample_data[samples[id].start], NULL);
                      gsl_data_handle_open (dhandle);

                      int fd = open (filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                      if (fd < 0)
                        {
                          BseErrorType error = bse_error_from_errno (errno, BSE_ERROR_FILE_OPEN_FAILED);
                          sfi_error ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
                        }

                      int xerrno = gsl_data_handle_dump_wav (dhandle, fd, 16, dhandle->setup.n_channels, (guint) dhandle->setup.mix_freq);
                      if (xerrno)
                        {
                          BseErrorType error = bse_error_from_errno (xerrno, BSE_ERROR_FILE_WRITE_FAILED);
                          sfi_error ("export to file %s failed: %s", filename.c_str(), bse_error_blurb (error));
                        }
                      close (fd);

                      xsystem (Birnet::string_printf ("smenc -m %d -O1 %s %s", midi_note, filename.c_str(), smname.c_str()));
                      if (sample_modes & 1)
                        {
                          xsystem (Birnet::string_printf ("smextract %s tail-loop", smname.c_str()));
                        }
                      xsystem (Birnet::string_printf ("smstrip %s", smname.c_str()));
                      xsystem (Birnet::string_printf ("smwavset add %s.smset %d %s", preset_xname.c_str(), midi_note, smname.c_str()));
                    }
                }
              xsystem (Birnet::string_printf ("smwavset link %s.smset", preset_xname.c_str()));
            }
        }
    }
  return 0;
}
