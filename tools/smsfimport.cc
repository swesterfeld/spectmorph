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

#include <vector>

#include "smgenericin.hh"
#include <stdlib.h>

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

struct Preset
{
  string name;
  int    preset;
  int    bank;
  int    preset_bag_index;
};

struct BagEntry
{
  int gen_index;
  int mod_index;
};

vector<Preset> presets;
vector<BagEntry> preset_bag;

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
      debug ("generator %d\n", read_ui16 (in));
      debug ("amount %d\n", read_ui16 (in));
      len -= 4;
    }
}

void
read_inst (GenericIn *in, int len)
{
  debug ("inst len = %d\n", len);
  while (len >= 22)
    {
      debug ("instname %s\n", read_fixed_string (in, 20).c_str());
      debug ("bagindex %d\n", read_ui16 (in));
      len -= 22;
    }
}

void
read_ibag (GenericIn *in, int len)
{
  debug ("ibag len = %d\n", len);
  while (len >= 4)
    {
      debug ("instgenidx %d\n", read_ui16 (in));
      debug ("instmodidx %d\n", read_ui16 (in));
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
      debug ("generator %d\n", read_ui16 (in));
      debug ("amount %d\n", read_ui16 (in));
      len -= 4;
    }
}

void
read_shdr (GenericIn *in, int len)
{
  debug ("igen len = %d\n", len);
  while (len >= 46)
    {
      debug ("samplename %s\n", read_fixed_string (in, 20).c_str());
      debug ("start %d\n", read_ui32 (in));
      debug ("end %d\n", read_ui32 (in));
      debug ("startloop %d\n", read_ui32 (in));
      debug ("endloop %d\n", read_ui32 (in));
      debug ("srate %d\n", read_ui32 (in));
      debug ("origpitch %d\n", in->get_byte());
      debug ("pitchcorrect %d\n", in->get_byte());
      debug ("samplelink %d\n", read_ui16 (in));
      debug ("sampletype %d\n", read_ui16 (in));
      len -= 46;
    }
}

int
main (int argc, char **argv)
{
  assert (argc == 2);

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
          int start = pi->preset_bag_index, end = (pi + 1)->preset_bag_index;
          for (vector<BagEntry>::iterator bi = preset_bag.begin() + start; bi != preset_bag.begin() + end; bi++)
            {
              printf ("    genndx %d\n", bi->gen_index);
              printf ("    modndx %d\n", bi->mod_index);
            }
        }
    }
  return 0;
}
