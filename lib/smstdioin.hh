// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_STDIO_IN_HH
#define SPECTMORPH_STDIO_IN_HH

#include <string>
#include "smgenericin.hh"
#include "smleakdebugger.hh"

namespace SpectMorph
{

class StdioIn final : public GenericIn
{
  LeakDebugger leak_debugger { "SpectMorph::StdioIn" };

  FILE        *file;
  std::string  filename;
  size_t file_size;
  size_t file_pos = 0;

  StdioIn (FILE *file, const std::string& filename, size_t file_size);
public:
  ~StdioIn();

  static GenericInP open (const std::string& filename);

  int get_byte() override;     // like fgetc
  size_t read (void *ptr, size_t size) override;
  bool skip (size_t size) override;
  const unsigned char *mmap_mem (size_t& remaining) override;
  size_t get_pos() override;
  bool get_remaining (size_t& remaining) override;
  GenericInP open_subfile (size_t pos, size_t len) override;
};

}

#endif /* SPECTMORPH_STDIO_IN_HH */
