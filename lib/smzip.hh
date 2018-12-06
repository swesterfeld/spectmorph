// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_ZIP_HH
#define SPECTMORPH_ZIP_HH

#include <string>
#include <vector>

namespace SpectMorph
{

class ZipReader
{
  void    *reader = nullptr;
  bool     need_close = false;
  int32_t  m_error = 0;
public:
  ZipReader (const std::string& filename);
  ~ZipReader();

  std::vector<std::string>  filenames();
  int32_t                   error() const;
  std::vector<uint8_t>      read (const std::string& name);
};

class ZipWriter
{
  void    *writer = nullptr;
  bool     need_close = false;
  int32_t  m_error = 0;
public:
  ZipWriter (const std::string& filename);
  ~ZipWriter();
  void    add (const std::string& filename, const std::vector<uint8_t>& data);
  void    add (const std::string& filename, const std::string& text);
  int32_t error() const;
};

}

#endif
