// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavdata.hh"
#include "smmath.hh"

#include <sndfile.h>
#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

static string
strip_dot (string s)
{
  if (!s.empty() && s[s.size() - 1] == '.')
    s.erase (s.size() - 1);
  return s;
}

WavData::WavData()
{
  clear();
}

void
WavData::clear()
{
  m_samples.clear();

  m_n_channels  = 0;
  m_bit_depth   = 0;
  m_mix_freq    = 0;
  m_error_blurb = "";
}

bool
WavData::load (const string& filename)
{
  return load ([&] (SF_INFO *sfinfo) {
    return sf_open (filename.c_str(), SFM_READ, sfinfo);
  });
}

bool
WavData::load (std::function<SNDFILE* (SF_INFO *)> open_func)
{
  SF_INFO sfinfo = { 0, };

  SNDFILE *sndfile = open_func (&sfinfo);

  int error = sf_error (sndfile);
  if (error)
    {
      m_error_blurb = strip_dot (sf_strerror (sndfile));
      if (sndfile)
        sf_close (sndfile);

      return false;
    }

  vector<int> isamples (sfinfo.frames * sfinfo.channels);
  sf_count_t count = sf_readf_int (sndfile, &isamples[0], sfinfo.frames);

  error = sf_error (sndfile);
  if (error)
    {
      m_error_blurb = strip_dot (sf_strerror (sndfile));
      sf_close (sndfile);

      return false;
    }

  if (count != sfinfo.frames)
    {
      m_error_blurb = "Reading sample data failed: short read";
      sf_close (sndfile);

      return false;
    }

  m_samples.resize (sfinfo.frames * sfinfo.channels);

  /* reading a wav file and saving it again with the libsndfile float API will
   * change some values due to normalization issues:
   *   http://www.mega-nerd.com/libsndfile/FAQ.html#Q010
   *
   * to avoid the problem, we use the int API and do the conversion beween int
   * and float manually - the important part is that the normalization factors
   * used during read and write are identical
   */
  const double norm = 1.0 / 0x80000000LL;
  for (size_t i = 0; i < m_samples.size(); i++)
    m_samples[i] = isamples[i] * norm;

  m_mix_freq    = sfinfo.samplerate;
  m_n_channels  = sfinfo.channels;

  switch (sfinfo.format & SF_FORMAT_SUBMASK)
    {
      case SF_FORMAT_PCM_U8:
      case SF_FORMAT_PCM_S8:
          m_bit_depth = 8;
          break;

      case SF_FORMAT_PCM_16:
          m_bit_depth = 16;
          break;

      case SF_FORMAT_PCM_24:
          m_bit_depth = 24;
          break;

      case SF_FORMAT_FLOAT:
      case SF_FORMAT_PCM_32:
          m_bit_depth = 32;
          break;

      case SF_FORMAT_DOUBLE:
          m_bit_depth = 64;
          break;

      default:
          m_bit_depth = 32; /* unknown */
    }

  error = sf_close (sndfile);
  if (error)
    {
      m_error_blurb = strip_dot (sf_error_number (error));
      return false;
    }
  return true;
}

bool
WavData::load_mono (const string& filename)
{
  if (!load (filename))
    return false;

  if (m_n_channels != 1)
    {
      m_error_blurb = "Only mono files supported";
      return false;
    }

  return true;
}

bool
WavData::save (const string& filename, OutFormat out_format)
{
  return save ([&] (SF_INFO *sfinfo)
    {
      return sf_open (filename.c_str(), SFM_WRITE, sfinfo);
    }, out_format);
}

namespace {
struct VirtualData
{
  vector<unsigned char> *mem    = nullptr;
  sf_count_t             offset = 0;
};
}

static sf_count_t
virtual_get_len (void *data)
{
  VirtualData *vdata = static_cast<VirtualData *> (data);

  return vdata->mem->size();
}

static sf_count_t
virtual_seek (sf_count_t offset, int whence, void *data)
{
  VirtualData *vdata = static_cast<VirtualData *> (data);

  if (whence == SEEK_CUR)
    {
      vdata->offset = vdata->offset + offset;
    }
  else if (whence == SEEK_SET)
    {
      vdata->offset = offset;
    }
  else if (whence == SEEK_END)
    {
      vdata->offset = vdata->mem->size() + offset;
    }

  /* can't seek beyond eof */
  vdata->offset = sm_bound<sf_count_t> (0, vdata->offset, vdata->mem->size());
  return vdata->offset;
}

static sf_count_t
virtual_read (void *ptr, sf_count_t count, void *data)
{
  /* FIXME: need to implement reading, too */
  VirtualData *vdata = static_cast<VirtualData *> (data);

  int rcount = 0;
  unsigned char *uptr = static_cast<unsigned char *> (ptr);
  for (sf_count_t i = 0; i < count; i++)
    {
      size_t rpos = i + vdata->offset;
      if (rpos < vdata->mem->size())
        {
          uptr[i] = (*vdata->mem)[rpos];
          rcount++;
        }
    }
  vdata->offset += rcount;
  return rcount;
}

static sf_count_t
virtual_write (const void *ptr, sf_count_t count, void *data)
{
  VirtualData *vdata = static_cast<VirtualData *> (data);

  const unsigned char *uptr = static_cast<const unsigned char *> (ptr);
  for (sf_count_t i = 0; i < count; i++)
    {
      unsigned char ch = uptr[i];

      size_t wpos = i + vdata->offset;
      if (wpos >= vdata->mem->size())
        vdata->mem->resize (wpos + 1);
      (*vdata->mem)[wpos] = ch;
    }
  vdata->offset += count;
  return count;
}

static sf_count_t
virtual_tell (void *data)
{
  VirtualData *vdata = static_cast<VirtualData *> (data);
  return vdata->offset;
}

bool
WavData::save (vector<unsigned char>& out, OutFormat out_format)
{
  VirtualData virtual_data;

  virtual_data.mem = &out;

  SF_VIRTUAL_IO sfvirtual = {
    virtual_get_len,
    virtual_seek,
    virtual_read,
    virtual_write,
    virtual_tell
  };
  return save ([&] (SF_INFO *sfinfo)
    {
      return sf_open_virtual (&sfvirtual, SFM_WRITE, sfinfo, &virtual_data);
    }, out_format);
}

bool
WavData::load (const vector<unsigned char>& in)
{
  VirtualData virtual_data;

  /* to ensure that in really isn't modified */
  vector<unsigned char> in_copy = in;
  virtual_data.mem = &in_copy;

  SF_VIRTUAL_IO sfvirtual = {
    virtual_get_len,
    virtual_seek,
    virtual_read,
    virtual_write,
    virtual_tell
  };
  return load ([&] (SF_INFO *sfinfo) {
    return sf_open_virtual (&sfvirtual, SFM_READ, sfinfo, &virtual_data);
  });
}

bool
WavData::save (std::function<SNDFILE* (SF_INFO *)> open_func, OutFormat out_format)
{
  SF_INFO sfinfo = {0,};

  sfinfo.samplerate = sm_round_positive (m_mix_freq);
  sfinfo.channels   = m_n_channels;

  switch (out_format)
  {
    case OutFormat::WAV:  sfinfo.format = SF_FORMAT_WAV;
                          break;
    case OutFormat::FLAC: sfinfo.format = SF_FORMAT_FLAC;
                          break;
    default:              assert (false);
  }
  if (m_bit_depth > 16)
    sfinfo.format |= SF_FORMAT_PCM_24;
  else
    sfinfo.format |= SF_FORMAT_PCM_16;

  SNDFILE *sndfile = open_func (&sfinfo);
  int error = sf_error (sndfile);
  if (error)
    {
      m_error_blurb = strip_dot (sf_strerror (sndfile));
      if (sndfile)
        sf_close (sndfile);

      return false;
    }

  vector<int> isamples (m_samples.size());
  for (size_t i = 0; i < m_samples.size(); i++)
    {
      const double norm      =  0x80000000LL;
      const double min_value = -0x80000000LL;
      const double max_value =  0x7FFFFFFF;

      isamples[i] = lrint (sm_bound<double> (min_value, m_samples[i] * norm, max_value));
    }

  sf_count_t frames = m_samples.size() / m_n_channels;
  sf_count_t count = sf_writef_int (sndfile, &isamples[0], frames);

  error = sf_error (sndfile);
  if (error)
    {
      m_error_blurb = strip_dot (sf_strerror (sndfile));
      sf_close (sndfile);

      return false;
    }

  if (count != frames)
    {
      m_error_blurb = "Writing sample data failed: short write";
      sf_close (sndfile);

      return false;
    }

  error = sf_close (sndfile);
  if (error)
    {
      m_error_blurb = strip_dot (sf_error_number (error));
      return false;
    }
  return true;
}

WavData::WavData (const vector<float>& samples, int n_channels, float mix_freq, int bit_depth)
{
  m_samples     = samples;
  m_n_channels  = n_channels;
  m_mix_freq    = mix_freq;
  m_bit_depth   = bit_depth;
}

void
WavData::load (const vector<float>& samples, int n_channels, float mix_freq, int bit_depth)
{
  // same function: WavData::WavData(...)

  m_samples     = samples;
  m_n_channels  = n_channels;
  m_mix_freq    = mix_freq;
  m_bit_depth   = bit_depth;
}

void
WavData::prepend (const vector<float>& samples)
{
  assert (samples.size() % m_n_channels == 0);

  m_samples.insert (m_samples.begin(), samples.begin(), samples.end());
}

float
WavData::operator[] (size_t pos) const
{
  assert (pos < m_samples.size());

  return m_samples[pos];
}

float
WavData::mix_freq() const
{
  return m_mix_freq;
}

int
WavData::n_channels() const
{
  return m_n_channels;
}

int
WavData::bit_depth() const
{
  return m_bit_depth;
}

const vector<float>&
WavData::samples() const
{
  return m_samples;
}

size_t
WavData::n_values() const
{
  return m_samples.size();
}

const char *
WavData::error_blurb() const
{
  return m_error_blurb.c_str();
}
