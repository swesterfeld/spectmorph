// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INSTENCCACHE_HH
#define SPECTMORPH_INSTENCCACHE_HH

#include "smwavdata.hh"
#include "smencoder.hh"
#include "sminstrument.hh"

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

  void        cache_save (const std::string& key);
  void        cache_try_load (const std::string& key, const std::string& need_version);

  InstEncCache(); // Singleton -> private constructor
public:
  Audio      *encode (const std::string& inst_name, const WavData& wav_data, int midi_note, int iclipstart, int iclipend, Instrument::EncoderConfig& cfg);
  void        clear();

  static InstEncCache *the(); // Singleton
};


}

#endif
