// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_ZIP_HH
#define SPECTMORPH_ZIP_HH

#include <string>
#include <vector>

#include "smutils.hh"

namespace SpectMorph
{

class ZipReader
{
  void                *reader = nullptr;
  bool                 need_close = false;
  int32_t              m_error = 0;
  void                *read_mem_stream = nullptr;
  std::vector<uint8_t> m_data;
public:
  ZipReader (const std::string& filename);
  ZipReader (const std::vector<uint8_t>& data);
  ~ZipReader();

  std::vector<std::string>  filenames();
  Error                     error() const;
  std::vector<uint8_t>      read (const std::string& name);

  static bool               is_zip (const std::string& name);
};

class ZipWriter
{
  void                 *writer = nullptr;
  bool                  need_close = false;
  int32_t               m_error = 0;
  void                 *write_mem_stream = nullptr;
public:
  enum class Compress { STORE = 0, DEFLATE };

  ZipWriter (const std::string& filename);
  ZipWriter();
  ~ZipWriter();
  void    add (const std::string& filename, const std::vector<uint8_t>& data, Compress compress = Compress::DEFLATE);
  void    add (const std::string& filename, const std::string& text, Compress compress = Compress::DEFLATE);
  void    close();
  Error   error() const;

  std::vector<uint8_t> data();
};

}

#endif
