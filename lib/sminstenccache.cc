// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "sminstenccache.hh"
#include "smbinbuffer.hh"
#include "sminstencoder.hh"
#include "smmmapin.hh"
#include "smmemout.hh"
#include "smmain.hh"
#include "smleakdebugger.hh"
#include "config.h"

#include <mutex>
#include <cinttypes>
#include <regex>

#include <assert.h>
#include <unistd.h>
#include <glib/gstdio.h>
#include <utime.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::map;
using std::regex;
using std::regex_search;

static string
cache_filename (const string& filename)
{
  return sm_get_user_dir (USER_DIR_CACHE) + "/" + filename;
}

static LeakDebugger leak_debugger ("SpectMorph::InstEncCache::CacheData");

InstEncCache::CacheData::CacheData()
{
  leak_debugger.add (this);
}

InstEncCache::CacheData::~CacheData()
{
  leak_debugger.del (this);
}

InstEncCache::InstEncCache() :
  cache_file_re ("inst_enc_[0-9a-f]{8}_[0-9a-f]{8}_[0-9]+_[0-9a-f]{40}$")
{
  delete_old_files();
}

InstEncCache*
InstEncCache::the()
{
  return Global::inst_enc_cache();
}

void
InstEncCache::cache_save_L (const string& key)
{
  const CacheData& cache_data = cache[key];

  BinBuffer buffer;

  buffer.write_start ("SpectMorphCache");
  buffer.write_string (cache_data.version.c_str());
  buffer.write_int (cache_data.data.size());
  buffer.write_string (sha1_hash (&cache_data.data[0], cache_data.data.size()).c_str());
  buffer.write_end();

  vector<string> files;
  Error error = read_dir (sm_get_user_dir (USER_DIR_CACHE), files);
  for (auto filename : files)
    {
      if (regex_search (filename, cache_file_re)) /* avoid unlink on something that we shouldn't delete */
        {
          if (filename.size() > key.size() && filename.compare (0, key.size(), key) == 0)
            {
              unlink (cache_filename (filename).c_str());
            }
        }
    }

  string out_filename = cache_filename (key) + "_" + cache_data.version;
  FILE *outf = fopen (out_filename.c_str(), "wb");
  if (outf)
    {
      for (auto ch : buffer.to_string())
        fputc (ch, outf);
      fputc (0, outf);
      const unsigned char *ptr = cache_data.data.data();
      fwrite (ptr, 1, cache_data.data.size(), outf);
      fclose (outf);
    }
}

static bool
ends_with (const string& value, const string& ending)
{
  if (ending.size() > value.size())
    return false;

  return std::equal (ending.rbegin(), ending.rend(), value.rbegin());
}

void
InstEncCache::cache_try_load_L (const string& cache_key, const string& need_version)
{
  GenericIn *in_file = nullptr;
  string     abs_filename;

  vector<string> files;
  Error error = read_dir (sm_get_user_dir (USER_DIR_CACHE), files);
  for (auto filename : files)
    {
      if (regex_search (filename, cache_file_re))
        {
          if (ends_with (filename, need_version))
            {
              abs_filename = cache_filename (filename);
              in_file = GenericIn::open (abs_filename);
              if (in_file)
                break;
            }
        }
    }

  if (!in_file)  // no cache entry
    return;

  // read header (till zero char)
  string header_str;
  int ch;
  while ((ch = in_file->get_byte()) > 0)
    header_str += char (ch);

  BinBuffer buffer;
  buffer.from_string (header_str);

  string type       = buffer.read_start_inplace();
  string version    = buffer.read_string_inplace();
  int    data_size  = buffer.read_int();
  string data_hash  = buffer.read_string_inplace();

  if (version == need_version)
    {
      vector<unsigned char> data (data_size);
      if (in_file->read (&data[0], data.size()) == data_size)
        {
          string load_data_hash = sha1_hash (&data[0], data.size());
          if (load_data_hash == data_hash)
            {
              cache[cache_key].version = version;
              cache[cache_key].data    = std::move (data);

              /* bump mtime on successful load; this information is used during
               * InstEncCache::delete_old_files() to remove the oldest cache files
               */
              g_utime (abs_filename.c_str(), nullptr);
            }
        }
    }
  delete in_file;
}

static string
mk_version (const string& wav_data_hash, int midi_note, int iclipstart, int iclipend, Instrument::EncoderConfig& cfg)
{
  /* create one single string that lists all the dependencies for the cache entry;
   * hash it to get a compact representation of the "version"
   */
  string depends;

  depends += wav_data_hash + "\n";
  depends += string_printf ("%s\n", PACKAGE_VERSION);
  depends += string_printf ("%d\n", midi_note);
  depends += string_printf ("%d\n", iclipstart);
  depends += string_printf ("%d\n", iclipend);
  if (cfg.enabled)
    {
      for (auto entry : cfg.entries)
        depends += entry.param + "=" + entry.value + "\n";
    }

  return sha1_hash (depends);
}

Audio *
InstEncCache::encode (Group *group, const WavData& wav_data, const string& wav_data_hash, int midi_note, int iclipstart, int iclipend, Instrument::EncoderConfig& cfg,
                      const std::function<bool()>& kill_function)
{
  // if group is not specified we create a random group just for this one request
  std::unique_ptr<Group> random_group;
  if (!group)
    {
      random_group.reset (create_group());
      group = random_group.get();
    }

  string cache_key = string_printf ("inst_enc_%s_%d", group->id.c_str(), midi_note);
  string version   = mk_version (wav_data_hash, midi_note, iclipstart, iclipend, cfg);

  // search disk cache and memory cache
  Audio *audio = cache_lookup (cache_key, version);
  if (audio)
    return audio;

  /* clip sample */
  vector<float> clipped_samples = wav_data.samples();

  /* sanity checks for clipping boundaries */
  iclipend   = sm_bound<int> (0, iclipend,  clipped_samples.size());
  iclipstart = sm_bound<int> (0, iclipstart, iclipend);

  /* do the clipping */
  clipped_samples.erase (clipped_samples.begin() + iclipend, clipped_samples.end());
  clipped_samples.erase (clipped_samples.begin(), clipped_samples.begin() + iclipstart);

  WavData wav_data_clipped (clipped_samples, 1, wav_data.mix_freq(), wav_data.bit_depth());

  InstEncoder enc;
  audio = enc.encode (wav_data_clipped, midi_note, cfg, kill_function);
  if (!audio)
    return nullptr;

  cache_add (cache_key, version, audio);

  return audio;
}

Audio *
InstEncCache::cache_lookup (const string& cache_key, const string& version)
{
  std::lock_guard<std::mutex> lg (cache_mutex);
  if (cache[cache_key].version != version)
    {
      cache_try_load_L (cache_key, version);
    }
  if (cache[cache_key].version == version) // cache hit (in memory)
    {
      vector<unsigned char>& data = cache[cache_key].data;
      cache[cache_key].read_stamp = cache_read_stamp++;

      GenericIn *in = MMapIn::open_mem (&data[0], &data[data.size()]);
      Audio     *audio = new Audio;
      Error      error = audio->load (in);

      delete in;

      if (!error)
        return audio;

      delete audio;
    }
  return nullptr;
}

void
InstEncCache::cache_add (const string& cache_key, const string& version, const Audio *audio)
{
  vector<unsigned char> data;
  MemOut                audio_mem_out (&data);

  audio->save (&audio_mem_out);

  // LOCK cache: store entry
  std::lock_guard<std::mutex> lg (cache_mutex);

  cache[cache_key].version    = version;
  cache[cache_key].data       = data;
  cache[cache_key].read_stamp = cache_read_stamp++;

  cache_save_L (cache_key);

  /* enforce size limits and expire cache data from time to time */
  if ((cache_read_stamp % 10) == 0)
    {
      delete_old_files();
      delete_old_memory_L();
    }
}

void
InstEncCache::clear()
{
  std::lock_guard<std::mutex> lg (cache_mutex);

  cache.clear();
}

InstEncCache::Group *
InstEncCache::create_group()
{
  Group *g = new Group();

  g->id = string_printf ("%08x_%08x", g_random_int(), g_random_int());
  return g;
}

void
InstEncCache::delete_old_files()
{
  struct Status
  {
    string abs_filename;
    uint64 mtime = 0;
    size_t size = 0;
  };
  vector<string> files;
  vector<Status> file_status;

  Error error = read_dir (sm_get_user_dir (USER_DIR_CACHE), files);
  if (error)
    return;

  for (auto filename : files)
    {
      Status status;
      status.abs_filename = cache_filename (filename);
      GStatBuf stbuf;
      if (g_stat (status.abs_filename.c_str(), &stbuf) == 0)
        {
          status.mtime = stbuf.st_mtime;
          status.size  = stbuf.st_size;;
          file_status.push_back (status);
        }
    }
  std::sort (file_status.begin(), file_status.end(),
    [](const Status& st1, const Status& st2)
      {
        /* sort: start with newest entries */
        return st1.mtime > st2.mtime;
      });

  const size_t max_total_size = 100 * 1000 * 1000; // 100 MB total cache size
  size_t total_size = 0;
  for (auto status : file_status)
    {
      /* using a regexp here avoids deleting unrelated files; even if something is
       * misconfigured this should make calling unlink() relatively safe */
      if (regex_search (status.abs_filename, cache_file_re))
        {
          total_size += status.size;
          if (total_size > max_total_size)
            unlink (status.abs_filename.c_str());
          // printf ("%s %" PRIu64 " %zd %zd\n", status.abs_filename.c_str(), status.mtime, status.size, total_size);
        }
    }
}

void
InstEncCache::delete_old_memory_L()
{
  struct Status
  {
    string key;
    uint64 read_stamp;
    size_t size;
  };
  vector<Status> mem_status;

  for (auto& entry : cache)
    {
      const string&     key = entry.first;
      const CacheData&  cache_data = entry.second;

      Status status;
      status.key        = key;
      status.size       = cache_data.data.size();
      status.read_stamp = cache_data.read_stamp;

      mem_status.push_back (status);
    }
  std::sort (mem_status.begin(), mem_status.end(),
    [](const Status& st1, const Status& st2)
      {
        /* sort: start with newest entries */
        return st1.read_stamp > st2.read_stamp;
      });

  const size_t max_total_size = 50 * 1000 * 1000; // 50 MB total cache size
  size_t total_size = 0;
  for (const auto& status : mem_status)
    {
      total_size += status.size;
      if (total_size > max_total_size)
        {
          /* since there is also disk cache, deletion from memory cache
           * will not affect performance much, as long as it can be reloaded
           * from the files we have stored
           */
          cache.erase (status.key);
        }
      // printf ("%s %" PRIu64 " %zd %zd\n", status.key.c_str(), status.read_stamp, status.size, total_size);
    }
}
