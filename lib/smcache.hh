// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_CACHE_HH
#define SPECTMORPH_CACHE_HH

#include "smwavset.hh"
#include <memory>
#include <map>
#include <mutex>

namespace SpectMorph
{

class CacheEntry
{
  std::shared_ptr<WavSet> m_wav_set;
  std::mutex              mutex;
public:

  bool                    used;

  std::shared_ptr<WavSet>
  wav_set()
  {
    std::lock_guard<std::mutex> lg (mutex);

    return m_wav_set;
  }
  void
  set_wav_set (std::shared_ptr<WavSet> wav_set)
  {
    std::lock_guard<std::mutex> lg (mutex);

    m_wav_set = wav_set;
  }
};

class Cache
{
  std::map<std::string, CacheEntry *> cache_map;
  std::mutex mutex;

public:
  void unset_used();
  void free_unused();

  static Cache *the(); // Singleton

  CacheEntry *lookup (const std::string& key);
  void        store (const std::string& key, CacheEntry *value);
};

}

#endif
