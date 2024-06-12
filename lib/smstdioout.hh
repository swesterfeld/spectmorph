// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_STDIO_OUT_HH
#define SPECTMORPH_STDIO_OUT_HH

#include <string>
#include "smgenericout.hh"
#include "smleakdebugger.hh"

namespace SpectMorph
{

class StdioOut final : public GenericOut
{
  LeakDebugger leak_debugger { "SpectMorph::StdioOut" };

  FILE *file;

  StdioOut (FILE *file);
public:
  static GenericOutP open (const std::string& filename);

  ~StdioOut();
  int put_byte (int c) override;
  int write (const void *ptr, size_t size) override;
};

}

#endif /* SPECTMORPH_STDIO_OUT_HH */
