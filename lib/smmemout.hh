// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MEM_OUT_HH
#define SPECTMORPH_MEM_OUT_HH

#include <string>
#include <vector>
#include "smgenericout.hh"
#include "smleakdebugger.hh"

namespace SpectMorph
{

class MemOut final : public GenericOut
{
  LeakDebugger leak_debugger { "SpectMorph::MemOut" };

  std::vector<unsigned char> *output;

  MemOut (std::vector<unsigned char> *output);
public:
  static GenericOutP open (std::vector<unsigned char> *output);

  int put_byte (int c) override;
  int write (const void *ptr, size_t size) override;
};

}

#endif /* SPECTMORPH_MEM_OUT_HH */
