// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_WAVSET_REPO_HH
#define SPECTMORPH_WAVSET_REPO_HH

#include "smwavset.hh"

#include <mutex>

#include <unordered_map>

namespace SpectMorph
{

class WavSetRepo {
  std::mutex mutex;
  std::unordered_map<std::string, WavSet *> wav_set_map;
public:
  ~WavSetRepo();

  WavSet *get (const std::string& filename);

  static WavSetRepo *the(); // Singleton
};

}

#endif
