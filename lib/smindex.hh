// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INDEX_HH
#define SPECTMORPH_INDEX_HH

#include <string>
#include <vector>

namespace SpectMorph
{

enum IndexType
{
  INDEX_INSTRUMENTS_DIR,
  INDEX_FILENAME,
  INDEX_NOT_DEFINED
};

class Index
{
public:
  struct Instrument
  {
    std::string smset;
    std::string label;
  };

  struct Group
  {
    std::string group;
    std::vector<Instrument> instruments;
  };

private:
  std::vector<std::string> m_smsets;
  std::string              m_smset_dir;
  std::vector<Group>       m_groups;

  std::string              m_expanded_filename;
  std::string              m_filename;
  std::string              m_dir;
  bool                     m_load_ok;

public:
  Index();

  void clear();
  bool load_file (const std::string& filename);

  IndexType   type() const;
  std::string filename() const;
  std::string expanded_filename() const;
  std::string dir() const;
  bool        load_ok() const;
  std::string label_to_smset (const std::string& label) const;
  std::string smset_to_label (const std::string& smset) const;

  const std::vector<Group>&       groups() const;

  const std::vector<std::string>& smsets() const;
  std::string                     smset_dir() const;
};

}

#endif
