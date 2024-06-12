// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smgenericin.hh"
#include "smmmapin.hh"
#include "smstdioin.hh"

using std::string;

namespace SpectMorph
{

/**
 * Open a file for reading, using memory mapping if possible, stdio based reading otherwise.
 *
 * \returns the newly created GenericIn object, or NULL on error
 */
GenericInP
GenericIn::open (const std::string& filename)
{
  GenericInP file = MMapIn::open (filename);
  if (!file)
    file = StdioIn::open (filename);
  return file;
}

GenericIn::~GenericIn()
{
}

}
