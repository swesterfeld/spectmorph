// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavdata.hh"

#include <sndfile.h>

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
