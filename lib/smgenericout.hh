// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_GENERIC_OUT_HH
#define SPECTMORPH_GENERIC_OUT_HH

#include <string>

namespace SpectMorph
{

/**
 * \brief Generic Output Stream
 *
 * This class is the abstract base class for different (binary) output streams
 * like "to memory", "to file", ...
 */
class GenericOut
{
public:
  virtual ~GenericOut();

  /**
   * Write one character into the output stream.
   *
   * \param c character to be written
   * \returns the character written, or EOF on error.
   */
  virtual int put_byte (int c) = 0;     // like fputc
  /**
   * Write a block of data into the stream.
   *
   * \param ptr pointer to the data to be written
   * \param size number of bytes to be written
   * \returns the number of bytes successfully written (can be 0)
   */
  virtual int write (const void *ptr, size_t size) = 0;
};

}

#endif /* SPECTMORPH_GENERIC_OUT_HH */
