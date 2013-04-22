// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smoutfile.hh"
#include "smstdioout.hh"

#include <glib.h>
#include <assert.h>

using std::string;
using std::vector;
using SpectMorph::OutFile;
using SpectMorph::GenericOut;
using SpectMorph::StdioOut;

OutFile::OutFile (const string& filename, const string& file_type, int file_version)
{
  file = StdioOut::open (filename);
  delete_file = true;
  write_file_type_and_version (file_type, file_version);
}

OutFile::OutFile (GenericOut *outfile, const string& file_type, int file_version)
{
  file = outfile;
  delete_file = false;
  write_file_type_and_version (file_type, file_version);
}

OutFile::~OutFile()
{
  if (file != NULL)
    {
      file->put_byte ('Z');  // eof
      if (delete_file)
        delete file;

      file = NULL;
    }
}

void
OutFile::write_file_type_and_version (const string& file_type, int file_version)
{
  if (file)
    {
      file->put_byte ('T');  // type
      write_raw_string (file_type);
      file->put_byte ('V');  // version
      write_raw_int (file_version);
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
OutFile::write_bool (const string& s,
                     bool          b)
{
  file->put_byte ('b'); // bool
  write_raw_string (s);
  file->put_byte (b ? 1 : 0);
}

void
OutFile::write_float_block (const string& s,
                            const vector<float>& fb)
{
  file->put_byte ('F');

  write_raw_string (s);
  write_raw_int (fb.size());

#if G_BYTE_ORDER != G_LITTLE_ENDIAN
  const int *fb_data = reinterpret_cast<const int *> (&fb[0]);

  vector<int> buffer (fb.size());
  for (size_t i = 0; i < fb.size(); i++)
    buffer[i] = GINT32_TO_LE (fb_data[i]); // little endian encoding

  file->write (&buffer[0], buffer.size() * 4);
#else
  file->write (&fb[0], fb.size() * 4);
#endif
}

void
OutFile::write_uint16_block (const string& s,
                            const vector<uint16_t>& ib)
{
  file->put_byte ('6');

  write_raw_string (s);
  write_raw_int (ib.size());

#if G_BYTE_ORDER != G_LITTLE_ENDIAN
  vector<int16_t> buffer (ib.size());
  for (size_t i = 0; i < ib.size(); i++)
    buffer[i] = GUINT16_TO_LE (ib[i]); // little endian encoding

  file->write (&buffer[0], buffer.size() * 2);
#else
  file->write (&ib[0], ib.size() * 2);
#endif
}

static string
blob_sum (const void *data, size_t size)
{
  char *sha256_sum = g_compute_checksum_for_data (G_CHECKSUM_SHA256, (const guchar *) data, size);
  string result = sha256_sum;
  g_free (sha256_sum);

  return result;
}

void
OutFile::write_blob (const string& s,
                     const void   *data,
                     size_t        size)
{
  file->put_byte ('O');    // BLOB => Object

  write_raw_string (s);

  string sha256_sum = blob_sum (data, size);
  if (stored_blobs.find (sha256_sum) != stored_blobs.end())
    {
      // a blob with the same data was stored before -> just store hash sum
      write_raw_int (-1);
      write_raw_string (sha256_sum);
    }
  else
    {
      // first time we store this blob
      write_raw_int (size);
      write_raw_string (sha256_sum);

      file->write (data, size);

      stored_blobs.insert (sha256_sum);
    }
}
