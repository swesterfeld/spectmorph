// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmmapin.hh"
#include "smutils.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

using std::vector;

using namespace SpectMorph;

GenericInP
MMapIn::open (const std::string& filename)
{
  if (getenv ("SPECTMORPH_NOMMAP"))
    return nullptr;

  GMappedFile *gmf = g_mapped_file_new (filename.c_str(), FALSE, nullptr);
  if (gmf)
    {
      unsigned char *bytes  = reinterpret_cast<unsigned char *> (g_mapped_file_get_contents (gmf));
      size_t         length = g_mapped_file_get_length (gmf);

      return GenericInP (new MMapIn (bytes, bytes + length, gmf));
    }
  else
    {
      return nullptr;
    }
}

GenericInP
MMapIn::open_vector (const vector<unsigned char>& vec)
{
  return GenericInP (new MMapIn (vec.data(), vec.data() + vec.size(), nullptr));
}

MMapIn::MMapIn (const unsigned char *mapfile, const unsigned char *mapend, GMappedFile *gmf) :
  mapfile (mapfile),
  mapend (mapend),
  g_mapped_file (gmf)
{
  pos = static_cast<const unsigned char *> (mapfile);
}

MMapIn::~MMapIn()
{
  if (g_mapped_file)
    g_mapped_file_unref (g_mapped_file);
}

int
MMapIn::get_byte()
{
  if (pos < mapend)
    return *pos++;
  else
    return EOF;
}

size_t
MMapIn::read (void *ptr, size_t size)
{
  if (!size || size > get_remaining())
    return 0;

  memcpy (ptr, pos, size);
  pos += size;
  return size;
}

bool
MMapIn::skip (size_t size)
{
  if (size > get_remaining())
    return false;

  if (size)
    pos += size;
  return true;
}

const unsigned char *
MMapIn::mmap_mem (size_t& remaining)
{
  remaining = get_remaining();
  return pos;
}

size_t
MMapIn::get_pos()
{
  return pos - mapfile;
}

GenericInP
MMapIn::open_subfile (size_t pos, size_t len)
{
  return GenericInP (new MMapIn (mapfile + pos, mapfile + pos + len, nullptr /* no mapped file */));
}

size_t
MMapIn::get_remaining()
{
  return pos == mapend ? 0 : mapend - pos;
}
