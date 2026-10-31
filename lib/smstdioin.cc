// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smstdioin.hh"
#include "smstdiosubin.hh"

#include <stdio.h>
#include <sys/stat.h>
#include <cstdint>
#include <limits>
#include <climits>
#include <assert.h>

using namespace SpectMorph;

using std::string;

GenericInP
StdioIn::open (const string& filename)
{
  FILE *file = fopen (filename.c_str(), "rb");

  if (!file)
    return nullptr;

  // Model files are treated as unchanged for the lifetime of this input.
  struct stat st;
  if (fstat (fileno (file), &st) != 0 || !S_ISREG (st.st_mode) || st.st_size < 0
      || uintmax_t (st.st_size) > std::numeric_limits<size_t>::max())
    {
      fclose (file);
      return nullptr;
    }

  return GenericInP (new StdioIn (file, filename, size_t (st.st_size)));
}

StdioIn::StdioIn (FILE *file, const string& filename, size_t file_size) :
  file (file),
  filename (filename),
  file_size (file_size)
{
}

StdioIn::~StdioIn()
{
  assert (file);
  fclose (file);
}

int
StdioIn::get_byte()
{
  const int result = fgetc (file);
  if (result != EOF)
    file_pos++;
  return result;
}

size_t
StdioIn::read (void *ptr, size_t size)
{
  const size_t read_bytes = fread (ptr, 1, size, file);
  file_pos += read_bytes;
  return read_bytes;
}

bool
StdioIn::skip (size_t size)
{
  if (size <= get_remaining() && size <= LONG_MAX
      && fseek (file, size, SEEK_CUR) == 0)
    {
      file_pos += size;
      return true;
    }
  return false;
}

const unsigned char*
StdioIn::mmap_mem (size_t& remaining)
{
  return NULL;
}

size_t
StdioIn::get_pos()
{
  return file_pos;
}

GenericInP
StdioIn::open_subfile (size_t pos, size_t len)
{
  return StdioSubIn::open (filename, pos, len);
}

size_t
StdioIn::get_remaining()
{
  return file_pos <= file_size ? file_size - file_pos : 0;
}
