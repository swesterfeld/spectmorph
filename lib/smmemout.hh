// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MEM_OUT_HH
#define SPECTMORPH_MEM_OUT_HH

#include <string>
#include <vector>
#include "smgenericout.hh"

namespace SpectMorph
{

class MemOut : public GenericOut
{
  std::vector<unsigned char> *output;

public:
  MemOut (std::vector<unsigned char> *output);
  ~MemOut();

  int put_byte (int c);
  int write (const void *ptr, size_t size);
};

}

#endif /* SPECTMORPH_MEM_OUT_HH */
