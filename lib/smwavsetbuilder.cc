// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavsetbuilder.hh"
#include "sminstencoder.hh"
#include "smbinbuffer.hh"

#include <mutex>

using namespace SpectMorph;

using std::string;
using std::map;
using std::vector;

static string
tmpfile (const string& filename)
{
  return sm_get_user_dir (USER_DIR_DATA) + "/" + filename;
}

WavSetBuilder::WavSetBuilder (const Instrument *instrument)
{
  for (size_t i = 0; i < instrument->size(); i++)
    {
      Sample *sample = instrument->sample (i);
      assert (sample);

      add_sample (sample);
    }
}

void
WavSetBuilder::add_sample (const Sample *sample)
{
  SampleData sd;

  sd.midi_note = sample->midi_note();

  // FIXME: clean this up
  sd.wav_data_ptr  = const_cast<WavData *> (&sample->wav_data);

  const double clip_adjust = std::max (0.0, sample->get_marker (MARKER_CLIP_START));

  sd.loop = sample->loop();
  sd.loop_start_ms = sample->get_marker (MARKER_LOOP_START) - clip_adjust;
  sd.loop_end_ms = sample->get_marker (MARKER_LOOP_END) - clip_adjust;
  sd.clip_start_ms = sample->get_marker (MARKER_CLIP_START);
  sd.clip_end_ms = sample->get_marker (MARKER_CLIP_END);

  sample_data_vec.push_back (sd);
}

class InstEncCache
{
  struct CacheData
  {
    string                version;
    vector<unsigned char> data;
  };
  void
  cache_save (const string& key, const CacheData& cache_data)
  {
    BinBuffer buffer;

    buffer.write_start ("SpectMorphCache");
    buffer.write_string (cache_data.version.c_str());
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
  cache_try_load (CacheData& cache_data, const string& key, const string& need_version)
  {
    FILE *inf = fopen (tmpfile (key).c_str(), "rb");

    if (!inf)  // no cache entry
      return;

    // read header (till zero char)
    string header_str;
    int ch;
    while ((ch = fgetc (inf)) > 0)
      header_str += char (ch);

    BinBuffer buffer;
    buffer.from_string (header_str);

    string type       = buffer.read_start_inplace();
    string version    = buffer.read_string_inplace();
    string data_hash  = buffer.read_string_inplace();

    printf ("type = %s\n", type.c_str());
    printf ("version = %s\n", version.c_str());
    printf ("need_version = %s\n", need_version.c_str());
    printf ("hash = %s\n", data_hash.c_str());

    vector<unsigned char> data;
    int c;
    while ((c = fgetc (inf)) >= 0)
      data.push_back (c);

    string load_data_hash = sha1_hash (&data[0], data.size());
    printf ("load size = %zd\n", data.size());
    printf ("load_hash = %s\n", load_data_hash.c_str());

    if (version == need_version && load_data_hash == data_hash)
      {
        cache_data.version = version;
        cache_data.data    = data;
      }
    fclose (inf);
  }
  string
  sha1_hash (const guchar *data, size_t len)
  {
    char *result = g_compute_checksum_for_data (G_CHECKSUM_SHA1, data, len);
    string hash = result;
    g_free (result);

    return hash;
  }
public:
  void
  encode (const WavData& wav_data, int midi_note, const string& filename)
  {
    static map<string, CacheData> cache;
    static std::mutex             cache_mutex;

    std::lock_guard<std::mutex> lg (cache_mutex); // more optimal range possible

    string cache_key = filename; // should be something like "Trumpet:55"
    string version = sha1_hash ((const guchar *) &wav_data.samples()[0], sizeof (float) * wav_data.samples().size());

    if (cache[cache_key].version != version)
      {
        cache_try_load (cache[cache_key], string_printf ("uni_cache_%d", midi_note), version);
      }
    if (cache[cache_key].version == version) // cache hit (in memory)
      {
        FILE *out = fopen (filename.c_str(), "wb");
        assert (out);

        for (auto b : cache[cache_key].data)
          fputc (b, out);
        fclose (out);
        printf ("... cache hit: %s\n", filename.c_str());
        return;
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
  }
};

void
WavSetBuilder::run()
{
  WavSet wav_set;

  for (auto& sd : sample_data_vec)
    {
      /* clipping */
      assert (sd.wav_data_ptr->n_channels() == 1);

      vector<float> samples = sd.wav_data_ptr->samples();
      vector<float> clipped_samples;
      for (size_t i = 0; i < samples.size(); i++)
        {
          double pos_ms = i * (1000.0 / sd.wav_data_ptr->mix_freq());
          if (pos_ms >= sd.clip_start_ms)
            {
              /* if we have a loop, the loop end determines the real end of the recording */
              if (sd.loop != Sample::Loop::NONE || pos_ms <= sd.clip_end_ms)
                clipped_samples.push_back (samples[i]);
            }
        }

      WavData wd_clipped;
      wd_clipped.load (clipped_samples, 1, sd.wav_data_ptr->mix_freq());

      string sm_name = tmpfile (string_printf ("x%d.sm", sd.midi_note));

      InstEncCache enc;
      enc.encode (wd_clipped, sd.midi_note, sm_name);

      WavSetWave new_wave;
      new_wave.midi_note = sd.midi_note;
      new_wave.path = sm_name;
      new_wave.channel = 0;
      new_wave.velocity_range_min = 0;
      new_wave.velocity_range_max = 127;

      wav_set.waves.push_back (new_wave);
    }
  wav_set.save (tmpfile ("x.smset"), true); // link wavset

  apply_loop_settings();
}

void
WavSetBuilder::apply_loop_settings()
{
  WavSet wav_set;

  wav_set.load (tmpfile ("x.smset"));

  // build index for sample data vector
  map<int, SampleData*> note_to_sd;
  for (auto& sd : sample_data_vec)
    note_to_sd[sd.midi_note] = &sd;

  for (auto& wave : wav_set.waves)
    {
      SampleData *sd = note_to_sd[wave.midi_note];

      if (!sd)
        {
          printf ("warning: no to sd mapping %d failed\n", wave.midi_note);
          continue;
        }

      Audio *audio = wave.audio;

      // FIXME! account for zero_padding at start of sample
      const int loop_start = sd->loop_start_ms / audio->frame_step_ms;
      const int loop_end   = sd->loop_end_ms / audio->frame_step_ms;


      if (sd->loop == Sample::Loop::NONE)
        {
          audio->loop_type = Audio::LOOP_NONE;
          audio->loop_start = 0;
          audio->loop_end = 0;
        }
      else if (sd->loop == Sample::Loop::FORWARD)
        {
          audio->loop_type = Audio::LOOP_FRAME_FORWARD;
          audio->loop_start = loop_start;
          audio->loop_end = loop_end;
        }
      else if (sd->loop == Sample::Loop::PING_PONG)
        {
          audio->loop_type = Audio::LOOP_FRAME_PING_PONG;
          audio->loop_start = loop_start;
          audio->loop_end = loop_end;
        }
      else if (sd->loop == Sample::Loop::SINGLE_FRAME)
        {
          audio->loop_type = Audio::LOOP_FRAME_FORWARD;

          // single frame loop
          audio->loop_start = loop_start;
          audio->loop_end   = loop_start;
        }

      string lt_string;
      bool have_loop_type = Audio::loop_type_to_string (audio->loop_type, lt_string);
      if (have_loop_type)
        printf ("loop-type  = %s [%d..%d]\n", lt_string.c_str(), audio->loop_start, audio->loop_end);
    }

  wav_set.save (tmpfile ("x.smset"));
}

void
WavSetBuilder::get_result (WavSet& wav_set)
{
  wav_set.load (tmpfile ("x.smset"));
}
