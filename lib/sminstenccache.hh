// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INSTENCCACHE_HH
#define SPECTMORPH_INSTENCCACHE_HH

#include "smwavdata.hh"
#include "smencoder.hh"
#include "sminstrument.hh"

#include <mutex>
#include <regex>

namespace SpectMorph
{

class InstEncCache
{
  struct CacheData
  {
    std::string                version;
    std::vector<unsigned char> data;
    uint64                     read_stamp = 0;

    CacheData();
    ~CacheData();
  };

  std::map<std::string, CacheData> cache;
  std::mutex                       cache_mutex;
  const std::regex                 cache_file_re;
  uint64                           cache_read_stamp = 0;

  void        cache_save_L (const std::string& key);
  void        cache_try_load_L (const std::string& key, const std::string& need_version);
  Audio      *cache_lookup (const std::string& cache_key, const std::string& version);
  void        cache_add (const std::string& cache_key, const std::string& version, const Audio *audio);

  void        delete_old_files();
  void        delete_old_memory_L();

public:
  class Group
  {
  public:
    std::string id;
  };

  Audio      *encode (Group *group, const WavData& wav_data, const std::string& wav_data_hash,
                      int midi_note, int iclipstart, int iclipend, Instrument::EncoderConfig& cfg,
                      const std::function<bool()>& kill_function);
  void        clear();
  Group      *create_group();

  InstEncCache();

  static InstEncCache *the(); // Singleton
};

}

#endif
