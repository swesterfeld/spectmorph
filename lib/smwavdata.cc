// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavdata.hh"
#include "smmath.hh"

#include <sndfile.h>
#include <assert.h>

using namespace SpectMorph;

using std::string;
using std::vector;

WavData::WavData()
{
  clear();
}

void
WavData::clear()
{
  m_samples.clear();

  m_n_channels  = 0;
  m_mix_freq    = 0;
  m_error_blurb = "";
}

bool
WavData::load (const string& filename)
{
  SF_INFO sfinfo = { 0, };

  SNDFILE *sndfile = sf_open (filename.c_str(), SFM_READ, &sfinfo);

  int error = sf_error (sndfile);
  if (error)
    {
      m_error_blurb = sf_error_number (error);
      if (sndfile)
        sf_close (sndfile);

      return false;
    }

  m_samples.resize (sfinfo.frames * sfinfo.channels);
  sf_count_t count = sf_readf_float (sndfile, &m_samples[0], sfinfo.frames);

  error = sf_error (sndfile);
  if (error)
    {
      m_error_blurb = sf_error_number (error);
      sf_close (sndfile);

      return false;
    }

  if (count != sfinfo.frames)
    {
      m_error_blurb = "reading sample data failed: short read";
      sf_close (sndfile);

      return false;
    }

  m_mix_freq    = sfinfo.samplerate;
  m_n_channels  = sfinfo.channels;

  error = sf_close (sndfile);
  if (error)
    {
      m_error_blurb = sf_error_number (error);
      return false;
    }
  return true;
}

bool
WavData::save (const string& filename)
{
  SF_INFO sfinfo = {0,};

  sfinfo.samplerate = sm_round_positive (m_mix_freq);
  sfinfo.channels   = m_n_channels;
  sfinfo.format     = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

  SNDFILE *sndfile = sf_open (filename.c_str(), SFM_WRITE, &sfinfo);
  int error = sf_error (sndfile);
  if (error)
    {
      m_error_blurb = sf_error_number (error);
      if (sndfile)
        sf_close (sndfile);

      return false;
    }

  sf_count_t frames = m_samples.size() / m_n_channels;
  sf_count_t count = sf_writef_float (sndfile, &m_samples[0], frames);

  error = sf_error (sndfile);
  if (error)
    {
      m_error_blurb = sf_error_number (error);
      sf_close (sndfile);

      return false;
    }

  if (count != frames)
    {
      m_error_blurb = "writing sample data failed: short write";
      sf_close (sndfile);

      return false;
    }

  error = sf_close (sndfile);
  if (error)
    {
      m_error_blurb = sf_error_number (error);
      return false;
    }
  return true;
}

WavData::WavData (const vector<float>& samples, int n_channels, float mix_freq)
{
  m_samples     = samples;
  m_n_channels  = n_channels;
  m_mix_freq    = mix_freq;
}

void
WavData::load (const vector<float>& samples, int n_channels, float mix_freq)
{
  // same function: WavData::WavData(...)

  m_samples     = samples;
  m_n_channels  = n_channels;
  m_mix_freq    = mix_freq;
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
