// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavloader.hh"
#include "smleakdebugger.hh"
#include "smwavdata.hh"

#include <stdio.h>

using namespace SpectMorph;
using std::vector;
using std::string;

WavLoader*
WavLoader::load (const string& filename)
{
  WavData wav_data;

  if (!wav_data.load (filename))
    {
      fprintf (stderr, "SpectMorph::WavLoader: can't open the input file %s: %s\n", filename.c_str(), wav_data.error_blurb());
      return NULL;
    }

  if (wav_data.n_channels() != 1)
    {
      fprintf (stderr, "SpectMorph::WavLoader: currently, only mono files are supported.\n");
      return NULL;
    }

  WavLoader *result = new WavLoader;

  result->m_samples  = wav_data.samples();
  result->m_mix_freq = wav_data.mix_freq();

  return result;
}

static LeakDebugger leak_debugger ("SpectMorph::WavLoader");

WavLoader::WavLoader()
{
  leak_debugger.add (this);
}

WavLoader::~WavLoader()
{
  leak_debugger.del (this);
}

const vector<float>&
WavLoader::samples()
{
  return m_samples;
}

double
WavLoader::mix_freq()
{
  return m_mix_freq;
}
