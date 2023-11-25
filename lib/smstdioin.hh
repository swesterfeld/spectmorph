// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_STDIO_IN_HH
#define SPECTMORPH_STDIO_IN_HH

#include <string>
#include "smgenericin.hh"

namespace SpectMorph
{

class StdioIn final : public GenericIn
{
  FILE        *file;
  std::string  filename;

  StdioIn (FILE *file, const std::string& filename);
  ~StdioIn();
public:
  static GenericIn* open (const std::string& filename);

  int get_byte() override;     // like fgetc
  int read (void *ptr, size_t size) override;
  bool skip (size_t size) override;
  const unsigned char *mmap_mem (size_t& remaining) override;
  size_t get_pos() override;
  GenericIn *open_subfile (size_t pos, size_t len) override;
};

}

#endif /* SPECTMORPH_STDIO_IN_HH */
