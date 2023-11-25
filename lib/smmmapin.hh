// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MMAP_IN_HH
#define SPECTMORPH_MMAP_IN_HH

#include <string>
#include <vector>
#include <glib.h>
#include "smgenericin.hh"

namespace SpectMorph
{

class MMapIn : public GenericIn
{
  const unsigned char *mapfile;
  const unsigned char *mapend;
  const unsigned char *pos;
  GMappedFile         *g_mapped_file;

  MMapIn (const unsigned char *mapfile, const unsigned char *mapend, GMappedFile *g_mapped_file);
  ~MMapIn();
public:
  static GenericIn* open (const std::string& filename);
  static GenericIn* open_vector (const std::vector<unsigned char>& vec);

  int get_byte();     // like fgetc
  int read (void *ptr, size_t size);
  bool skip (size_t size);
  const unsigned char *mmap_mem (size_t& remaining) override;
  size_t get_pos();
  GenericIn *open_subfile (size_t pos, size_t len);
};

}

#endif /* SPECTMORPH_MMAP_IN_HH */
