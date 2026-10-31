// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smstdiosubin.hh"

#include <stdio.h>
#include <sys/stat.h>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <climits>
#include <assert.h>

using namespace SpectMorph;

GenericInP
StdioSubIn::open (const std::string& filename, size_t pos, size_t len)
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

  if (pos > size_t (st.st_size) || pos > LONG_MAX || fseek (file, pos, SEEK_SET) != 0)
    {
      fclose (file);
      return nullptr;
    }

  return GenericInP (new StdioSubIn (file, pos, len, size_t (st.st_size)));
}

StdioSubIn::StdioSubIn (FILE *file, size_t pos, size_t len, size_t file_size) :
  file (file),
  file_pos (0),
  file_len (len),
  file_size (file_size),
  file_start (pos)
{
}

StdioSubIn::~StdioSubIn()
{
  assert (file);
  fclose (file);
}

int
StdioSubIn::get_byte()
{
  if (file_pos < file_len)
    {
      int result = fgetc (file);
      if (result != EOF)
        file_pos++;
      return result;
    }
  else
    {
      return EOF;
    }
}

size_t
StdioSubIn::read (void *ptr, size_t size)
{
  size = std::min (size, get_remaining());
  if (size)
    {
      const size_t read_bytes = fread (ptr, 1, size, file);
      file_pos += read_bytes;
      return read_bytes;
    }
  else
    {
      return 0;
    }
}

bool
StdioSubIn::skip (size_t size)
{
  if (size <= get_remaining() && size <= LONG_MAX)
    {
      if (fseek (file, size, SEEK_CUR) == 0)
        {
          file_pos += size;
          return true;
        }
    }
  return false;
}

const unsigned char*
StdioSubIn::mmap_mem (size_t& remaining)
{
  return NULL;
}

size_t
StdioSubIn::get_pos()
{
  return file_pos;
}

GenericInP
StdioSubIn::open_subfile (size_t pos, size_t len)
{
  assert (false); // implement me
  return nullptr;
}

size_t
StdioSubIn::get_remaining()
{
  if (file_pos > file_size - file_start)
    return 0;

  const size_t remaining = file_size - file_start - file_pos;
  return std::min (remaining, file_pos <= file_len ? file_len - file_pos : size_t (0));
}
