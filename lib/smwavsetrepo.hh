// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WAVSET_REPO_HH
#define SPECTMORPH_WAVSET_REPO_HH

#include "smwavset.hh"

#include <mutex>

#include <map>

namespace SpectMorph
{

class WavSetRepo {
  std::mutex mutex;
  std::map<std::string, WavSet *> wav_set_map;
public:
  ~WavSetRepo();

  WavSet *get (const std::string& filename);

  static WavSetRepo *the(); // Singleton
};

}

#endif
