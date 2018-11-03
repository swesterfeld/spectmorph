// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "sminstenccache.hh"
#include "smbinbuffer.hh"
#include "sminstencoder.hh"
#include "smmmapin.hh"

#include <mutex>

#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::map;

static string
tmpfile (const string& filename)
{
  return sm_get_user_dir (USER_DIR_DATA) + "/" + filename;
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

void
InstEncCache::cache_save (const string& key, const CacheData& cache_data)
{
  BinBuffer buffer;

  buffer.write_start ("SpectMorphCache");
  buffer.write_string (cache_data.version.c_str());
  buffer.write_int (cache_data.data.size());
  buffer.write_string (sha1_hash (&cache_data.data[0], cache_data.data.size()).c_str());
  buffer.write_end();

  string out_filename = tmpfile (key);
  FILE *outf = fopen (out_filename.c_str(), "wb");
  if (outf)
    {
      for (auto ch : buffer.to_string())
        fputc (ch, outf);
      fputc (0, outf);
      printf ("save size = %zd\n", cache_data.data.size());
      for (auto ch : cache_data.data)
        fputc (ch, outf);
      fclose (outf);
    }
}

void
InstEncCache::cache_try_load (CacheData& cache_data, const string& key, const string& need_version)
{
  GenericIn *in_file = GenericIn::open (tmpfile (key));

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
              cache_data.version = version;
              cache_data.data    = std::move (data);
            }
        }
    }
  delete in_file;
}


string
InstEncCache::sha1_hash (const guchar *data, size_t len)
{
  char *result = g_compute_checksum_for_data (G_CHECKSUM_SHA1, data, len);
  string hash = result;
  g_free (result);

  return hash;
}

Audio *
InstEncCache::encode (const WavData& wav_data, int midi_note, const string& filename)
{
  std::lock_guard<std::mutex> lg (cache_mutex); // more optimal range possible

  string cache_key = filename; // should be something like "Trumpet:55"
  string version = sha1_hash ((const guchar *) &wav_data.samples()[0], sizeof (float) * wav_data.samples().size());

  if (cache[cache_key].version != version)
    {
      cache_try_load (cache[cache_key], string_printf ("uni_cache_%d", midi_note), version);
    }
  if (cache[cache_key].version == version) // cache hit (in memory)
    {
      vector<unsigned char>& data = cache[cache_key].data;

      GenericIn *in = MMapIn::open_mem (&data[0], &data[data.size()]);
      Audio     *audio = new Audio;
      bool       load_ok = (audio->load (in) == Error::NONE);

      delete in;

      if (load_ok)
        return audio;

      delete audio;
      return nullptr;
    }

  InstEncoder enc;
  enc.encode (wav_data, midi_note, filename);

  vector<unsigned char> data;
  FILE *f = fopen (filename.c_str(), "rb");
  int c;
  while ((c = fgetc (f)) >= 0)
    data.push_back (c);
  fclose (f);

  cache[cache_key].version = version;
  cache[cache_key].data    = data;

  cache_save (string_printf ("uni_cache_%d", midi_note), cache[cache_key]);

  Audio *audio = new Audio;
  if (audio->load (filename) == Error::NONE)
    return audio;
  else
    {
      delete audio;
      return nullptr;
    }
}

void
InstEncCache::clear()
{
  std::lock_guard<std::mutex> lg (cache_mutex);

  cache.clear();
}
