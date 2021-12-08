// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_STDIO_OUT_HH
#define SPECTMORPH_STDIO_OUT_HH

#include <string>
#include "smgenericout.hh"

namespace SpectMorph
{

class StdioOut : public GenericOut
{
  FILE *file;

  StdioOut (FILE *file);
public:
  static GenericOut* open (const std::string& filename);

  ~StdioOut();
  int put_byte (int c);
  int write (const void *ptr, size_t size);
};

}

#endif /* SPECTMORPH_STDIO_OUT_HH */
