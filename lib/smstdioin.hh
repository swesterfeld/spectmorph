// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_STDIO_IN_HH
#define SPECTMORPH_STDIO_IN_HH

#include <string>
#include "smgenericin.hh"

namespace SpectMorph
{

class StdioIn : public GenericIn
{
  FILE        *file;
  std::string  filename;

  StdioIn (FILE *file, const std::string& filename);
  ~StdioIn();
public:
  static GenericIn* open (const std::string& filename);

  int get_byte();     // like fgetc
  int read (void *ptr, size_t size);
  bool skip (size_t size);
  unsigned char *mmap_mem (size_t& remaining);
  size_t get_pos();
  GenericIn *open_subfile (size_t pos, size_t len);
};

}

#endif /* SPECTMORPH_STDIO_IN_HH */
