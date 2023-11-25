// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_STDIO_SUB_IN_HH
#define SPECTMORPH_STDIO_SUB_IN_HH

#include <string>
#include "smgenericin.hh"

namespace SpectMorph
{

class StdioSubIn : public GenericIn
{
  FILE *file;
  size_t file_pos;
  size_t file_len;

  StdioSubIn (FILE *file, size_t pos, size_t len);
public:
  ~StdioSubIn();

  static GenericIn* open (const std::string& filename, size_t pos, size_t len);

  int get_byte();     // like fgetc
  int read (void *ptr, size_t size);
  bool skip (size_t size);
  const unsigned char *mmap_mem (size_t& remaining) override;
  size_t get_pos();
  GenericIn *open_subfile (size_t pos, size_t len);
};

}

#endif /* SPECTMORPH_STDIO_SUB_IN_HH */
