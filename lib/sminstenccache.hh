// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INSTENCCACHE_HH
#define SPECTMORPH_INSTENCCACHE_HH

#include "smwavdata.hh"
#include "smencoder.hh"

#include <mutex>

namespace SpectMorph
{

class InstEncCache
{
  struct CacheData
  {
    std::string                version;
    std::vector<unsigned char> data;
  };

  std::map<std::string, CacheData> cache;
  std::mutex                       cache_mutex;

  void        cache_save (const std::string& key, const CacheData& cache_data);
  void        cache_try_load (CacheData& cache_data, const std::string& key, const std::string& need_version);

  InstEncCache(); // Singleton -> private constructor
public:
  Audio      *encode (const WavData& wav_data, int midi_note, const std::string& filename);
  void        clear();

  static InstEncCache *the(); // Singleton
};


}

#endif
