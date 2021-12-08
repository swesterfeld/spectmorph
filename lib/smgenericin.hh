// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_GENERIC_IN_HH
#define SPECTMORPH_GENERIC_IN_HH

#include <string>

namespace SpectMorph
{

/**
 * \brief Generic Input Stream
 *
 * This class is the abstract base class for different (binary) input streams
 * like "from memory", "from file", ...
 */
class GenericIn
{
public:
  static GenericIn* open (const std::string& filename);

  virtual ~GenericIn();
  /**
   * Reads one byte from the input stream.
   *
   * \returns the byte if successful, or EOF on end of file
   */
  virtual int get_byte() = 0;     // like fgetc
  /**
   * Reads a block of binary input data.
   *
   * \param ptr pointer to the buffer for the data read
   * \param size number of bytes to be read
   *
   * \returns the number of bytes successfully read or 0; can be less than size on EOF/error
   */
  virtual int read (void *ptr, size_t size) = 0;
  /**
   * Skips a block of input data.
   *
   * \param size number of bytes to be skipped
   *
   * \returns true if the number of bytes was available and could be skipped, false otherwise
   */
  virtual bool skip (size_t size) = 0;
  /**
   * Get input stream position.
   *
   * \returns the position in the input stream
   */
  virtual size_t get_pos() = 0;
  /**
   * Try to access the memory of a memory-based input stream. This will fail for streams
   * that are not memory mapped. This function is useful because it allows writing faster
   * code than with read(); however, an alternative way of reading the data must be
   * used if this function fails.
   *
   * \param remaining is set to the number to the number of bytes that are available
   *
   * \returns pointer to the memory, or NULL if input is not memory mapped
   */
  virtual unsigned char *mmap_mem (size_t& remaining) = 0;
  /**
   * Open a new input stream that contains a part of the original file.
   *
   * \param pos start position in the input stream (offset like get_pos())
   * \param len length of the new subfile object
   *
   * \returns a GenericIn object with the subfile or NULL on error (open failed)
   */
  virtual GenericIn *open_subfile (size_t pos, size_t len) = 0;
};

}

#endif /* SPECTMORPH_GENERIC_IN_HH */
