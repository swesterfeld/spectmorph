// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavdata.hh"

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

  m_n_channels = 0;
  m_mix_freq   = 0;
}

Error
WavData::load (const string& filename)
{
  SF_INFO sfinfo = { 0, };

  SNDFILE *sndfile = sf_open (filename.c_str(), SFM_READ, &sfinfo);
  if (sf_error (sndfile) != 0)
    {
      return Error::FILE_NOT_FOUND; // FIXME
    }

  m_samples.resize (sfinfo.frames * sfinfo.channels);
  sf_count_t count = sf_readf_float (sndfile, &m_samples[0], sfinfo.frames);
  if (count != sfinfo.frames)
    return Error::FILE_NOT_FOUND; // FIXME; leak sndfile here

  m_mix_freq    = sfinfo.samplerate;
  m_n_channels  = sfinfo.channels;

  int err = sf_close (sndfile);
  if (err != 0)
    {
      return Error::FILE_NOT_FOUND; // FIXME
    }

  return Error::NONE;
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
