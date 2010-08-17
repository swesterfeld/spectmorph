#include <assert.h>
#include <stdio.h>

#include "smgenericin.hh"

using namespace SpectMorph;

using std::string;

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
read_i32 (GenericIn *in)
{
  int c0 = in->get_byte();
  int c1 = in->get_byte();
  int c2 = in->get_byte();
  int c3 = in->get_byte();

  return c0 + (c1 << 8) + (c2 << 16) + (c3 << 24);
}

int
read_i16 (GenericIn *in)
{
  int c0 = in->get_byte();
  int c1 = in->get_byte();

  return c0 + (c1 << 8);
}

void
read_ifil (GenericIn *in, int len)
{
  assert (len == 4);
  int major = read_i16 (in);
  int minor = read_i16 (in);
  printf ("format version %d.%d\n", major, minor);
}

void
read_INAM (GenericIn *in, int len)
{
  string result;

  while (len)
    {
      int b = in->get_byte();
      if (b == 0)
        assert (len == 1);
      else
        result += char (b);
      len--;
    }
  printf ("name '%s'\n", result.c_str());
}

void
read_isng (GenericIn *in, int len)
{
  string result;

  while (len)
    {
      int b = in->get_byte();
      if (b == 0)
        assert (len == 1);
      else
        result += char (b);
      len--;
    }
  printf ("target '%s'\n", result.c_str());
}

void
read_IPRD (GenericIn *in, int len)
{
  string result;

  while (len)
    {
      int b = in->get_byte();
      if (b == 0)
        assert (len == 1);
      else
        result += char (b);
      len--;
    }
  printf ("product '%s'\n", result.c_str());
}

void
read_IENG (GenericIn *in, int len)
{
  string result;

  while (len)
    {
      int b = in->get_byte();
      if (b == 0)
        assert (len == 1);
      else
        result += char (b);
      len--;
    }
  printf ("engineer '%s'\n", result.c_str());
}

void
read_ISFT (GenericIn *in, int len)
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
  printf ("tool '%s'\n", result.c_str());
}

void
read_ICRD (GenericIn *in, int len)
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
  printf ("creation date '%s'\n", result.c_str());
}

void
read_ICMT (GenericIn *in, int len)
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
  printf ("comment '%s'\n", result.c_str());
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

void
read_ICOP (GenericIn *in, int len)
{
  string result = read_string (in, len);
  printf ("copyright '%s'\n", result.c_str());
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

  int len = read_i32 (in);
  printf ("len = %d\n", len);

  fcc = read_fourcc (in);
  if (fcc != "sfbk")
    {
      fprintf (stderr, "missing sfbk chunk\n");
      return 1;
    }
  fcc = read_fourcc (in);
  printf ("fcc = %s\n", fcc.c_str());  
  len = read_i32 (in);

  printf ("len = %d\n", len);
  printf ("end pos = %zd\n", len + in->get_pos());

  size_t list_end_pos = len + in->get_pos();

  fcc = read_fourcc (in);
  printf ("fcc<info> = %s\n", fcc.c_str());  

  while (in->get_pos() < list_end_pos)
    {
      fcc = read_fourcc (in);
      len = read_i32 (in);

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
  printf ("fcc = %s\n", fcc.c_str());  
  len = read_i32 (in);
  printf ("len = %d\n", len);

#if 0
  in->skip (len);
  fcc = read_fourcc (in);
  printf ("fcc = %s\n", fcc.c_str());  
  len = read_i32 (in);
  printf ("len = %d\n", len);
#endif

  return 0;
}
