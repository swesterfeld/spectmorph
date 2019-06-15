// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "sminstenccache.hh"
#include "smbinbuffer.hh"
#include "sminstencoder.hh"
#include "smmmapin.hh"
#include "smmemout.hh"

#include <mutex>

#include <assert.h>
#include <unistd.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::map;

static string
cache_filename (const string& filename)
{
  return sm_get_user_dir (USER_DIR_CACHE) + "/" + filename;
}

InstEncCache::InstEncCache()
{
}

InstEncCache*
InstEncCache::the()
{
  static InstEncCache *instance = NULL;
  if (!instance)
    instance = new InstEncCache;

  return instance;
}

static Error
read_dir (const string& dirname, vector<string>& files)
{
  GError *gerror = nullptr;
  const char *filename;

  GDir *dir = g_dir_open (dirname.c_str(), 0, &gerror);
  if (gerror)
    {
      Error error (gerror->message);
      g_error_free (gerror);
      return error;
    }
  files.clear();
  while ((filename = g_dir_read_name (dir)))
    files.push_back (filename);
  g_dir_close (dir);

  return Error::Code::NONE;
}

void
InstEncCache::cache_save (const string& key)
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
      if (filename.size() > key.size() && filename.compare (0, key.size(), key) == 0)
        {
          unlink (cache_filename (filename).c_str());
        }
    }

  string out_filename = cache_filename (key) + "_" + cache_data.version;
  FILE *outf = fopen (out_filename.c_str(), "wb");
  if (outf)
    {
      for (auto ch : buffer.to_string())
        fputc (ch, outf);
      fputc (0, outf);
      for (auto ch : cache_data.data)
        fputc (ch, outf);
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
InstEncCache::cache_try_load (const string& cache_key, const string& need_version)
{
  GenericIn *in_file = nullptr;

  vector<string> files;
  Error error = read_dir (sm_get_user_dir (USER_DIR_CACHE), files);
  for (auto filename : files)
    {
      if (ends_with (filename, need_version))
        {
          in_file = GenericIn::open (cache_filename (filename));
          if (in_file)
            break;
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

  string cache_key = string_printf ("%s_%d", group->id.c_str(), midi_note);
  string version   = mk_version (wav_data_hash, midi_note, iclipstart, iclipend, cfg);

  // LOCK cache, look for entry
  {
    std::lock_guard<std::mutex> lg (cache_mutex);
    if (cache[cache_key].version != version)
      {
        cache_try_load (cache_key, version);
      }
    if (cache[cache_key].version == version) // cache hit (in memory)
      {
        vector<unsigned char>& data = cache[cache_key].data;

        GenericIn *in = MMapIn::open_mem (&data[0], &data[data.size()]);
        Audio     *audio = new Audio;
        Error      error = audio->load (in);

        delete in;

        if (!error)
          return audio;

        delete audio;
        return nullptr;
      }
  }

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
  Audio *audio = enc.encode (wav_data_clipped, midi_note, cfg, kill_function);
  if (!audio)
    return nullptr;

  vector<unsigned char> data;
  MemOut                audio_mem_out (&data);

  audio->save (&audio_mem_out);

  // LOCK cache: store entry
  {
    std::lock_guard<std::mutex> lg (cache_mutex);

    cache[cache_key].version = version;
    cache[cache_key].data    = data;

    cache_save (cache_key);
  }

  return audio;
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

  g->id = string_printf ("%08x-%08x", g_random_int(), g_random_int());
  return g;
}
