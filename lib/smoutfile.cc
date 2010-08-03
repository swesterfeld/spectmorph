/* 
 * Copyright (C) 2009-2010 Stefan Westerfeld
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

#include "smoutfile.hh"
#include "smstdioout.hh"

#include <assert.h>

using std::string;
using std::vector;
using SpectMorph::OutFile;
using SpectMorph::GenericOut;
using SpectMorph::StdioOut;

OutFile::OutFile (const string& filename, const string& file_type)
{
  file = StdioOut::open (filename);
  delete_file = true;
  write_file_type (file_type);
}

OutFile::OutFile (GenericOut *outfile, const string& file_type)
{
  file = outfile;
  delete_file = false;
  write_file_type (file_type);
}

OutFile::~OutFile()
{
  if (file != NULL)
    {
      if (delete_file)
        delete file;

      file = NULL;
    }
}

void
OutFile::write_file_type (const string& file_type)
{
  if (file)
    {
      file->put_byte ('T');  // type
      write_raw_string (file_type);
    }
}

void
OutFile::begin_section (const string& s)
{
  file->put_byte ('B'); // begin section
  write_raw_string (s);
}

void
OutFile::end_section()
{
  file->put_byte ('E'); // end section
}

void
OutFile::write_raw_string (const string& s)
{
  for (size_t i = 0; i < s.size(); i++)
    file->put_byte (s[i]);
  file->put_byte (0);
}

void
OutFile::write_raw_int (int i)
{
  // little endian encoding
  file->put_byte (i & 0xff);
  file->put_byte ((i >> 8) & 0xff);
  file->put_byte ((i >> 16) & 0xff);
  file->put_byte ((i >> 24) & 0xff);
}

void
OutFile::write_float (const string& s,
                      double f)
{
  union {
    float f;
    int i;
  } u;
  u.f = f;

  file->put_byte ('f'); // float

  write_raw_string (s);
  write_raw_int (u.i);
}

void
OutFile::write_int (const string& s,
                    int   i)
{
  file->put_byte ('i'); // int

  write_raw_string (s);
  write_raw_int (i);
}

void
OutFile::write_string (const string& s,
                       const string& data)
{
  file->put_byte ('s'); // string

  write_raw_string (s);
  write_raw_string (data);
}

void
OutFile::write_float_block (const string& s,
                            const vector<float>& fb)
{
  file->put_byte ('F');

  write_raw_string (s);
  write_raw_int (fb.size());

  vector<unsigned char> buffer (fb.size() * 4);
  size_t bpos = 0;
  for (size_t i = 0; i < fb.size(); i++)
    {
      union {
        float f;
        int i;
      } u;
      u.f = fb[i];
      // write_raw_int (u.i);

      // little endian encoding
      buffer[bpos++] = u.i;
      buffer[bpos++] = u.i >> 8;
      buffer[bpos++] = u.i >> 16;
      buffer[bpos++] = u.i >> 24;
    }
  assert (bpos == buffer.size());
  file->write (&buffer[0], buffer.size());
}

void
OutFile::write_blob (const string& s,
                     const void   *data,
                     size_t        size)
{
  file->put_byte ('O');    // BLOB => Object

  write_raw_string (s);
  write_raw_int (size);

  file->write (data, size);
}
