// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smgenericin.hh"
#include "smmmapin.hh"
#include "smstdioin.hh"

using SpectMorph::GenericIn;
using SpectMorph::MMapIn;
using SpectMorph::StdioIn;
using std::string;

/**
 * Open a file for reading, using memory mapping if possible, stdio based reading otherwise.
 *
 * \returns the newly created GenericIn object, or NULL on error
 */
GenericIn*
GenericIn::open (const std::string& filename)
{
  GenericIn *file = MMapIn::open (filename);
  if (!file)
    file = StdioIn::open (filename);
  return file;
}

GenericIn::~GenericIn()
{
}
