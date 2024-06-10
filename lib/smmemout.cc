// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmemout.hh"
#include <stdio.h>

using namespace SpectMorph;

using std::vector;

MemOut::MemOut (vector<unsigned char> *output)
  : output (output)
{
}

int
MemOut::put_byte (int c)
{
  output->push_back (c);
  return c;
}

int
MemOut::write (const void *ptr, size_t size)
{
  const unsigned char *uptr = reinterpret_cast<const unsigned char *> (ptr);
  output->insert (output->end(), uptr, uptr + size);
  return size;
}
