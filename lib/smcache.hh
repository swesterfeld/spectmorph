// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_CACHE_HH
#define SPECTMORPH_CACHE_HH

#include "smwavset.hh"
#include <memory>
#include <map>
#include <mutex>

namespace SpectMorph
{

struct CacheEntry
{
  bool                    used;
  std::shared_ptr<WavSet> wav_set;
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
