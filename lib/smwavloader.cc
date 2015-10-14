// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavloader.hh"
#include "smleakdebugger.hh"

#include <bse/bseloader.hh>
#include <stdio.h>

using namespace SpectMorph;
using std::vector;
using std::string;

WavLoader*
WavLoader::load (const string& filename)
{
  /* open input */
  Bse::ErrorType error;

  BseWaveFileInfo *wave_file_info = bse_wave_file_info_load (filename.c_str(), &error);
  if (!wave_file_info)
    {
      fprintf (stderr, "SpectMorph::WavLoader: can't open the input file %s: %s\n", filename.c_str(), bse_error_blurb (error));
      return NULL;
    }

  BseWaveDsc *waveDsc = bse_wave_dsc_load (wave_file_info, 0, FALSE, &error);
  if (!waveDsc)
    {
      fprintf (stderr, "SpectMorph::WavLoader: can't open the input file %s: %s\n", filename.c_str(), bse_error_blurb (error));
      return NULL;
    }

  GslDataHandle *dhandle = bse_wave_handle_create (waveDsc, 0, &error);
  if (!dhandle)
    {
      fprintf (stderr, "SpectMorph::WavLoader: can't open the input file %s: %s\n", filename.c_str(), bse_error_blurb (error));
      return NULL;
    }

  error = gsl_data_handle_open (dhandle);
  if (error)
    {
      fprintf (stderr, "SpectMorph::WavLoader: can't open the input file %s: %s\n", filename.c_str(), bse_error_blurb (error));
      return NULL;
    }

  if (gsl_data_handle_n_channels (dhandle) != 1)
    {
      fprintf (stderr, "SpectMorph::WavLoader: currently, only mono files are supported.\n");
      return NULL;
    }

  vector<float> block (1024);

  WavLoader *result = new WavLoader;

  const uint64 n_values = gsl_data_handle_length (dhandle);
  uint64 pos = 0;
  while (pos < n_values)
    {
      /* read data from file */
      uint64 r = gsl_data_handle_read (dhandle, pos, block.size(), &block[0]);

      for (uint64 t = 0; t < r; t++)
        result->m_samples.push_back (block[t]);
      pos += r;
    }
  result->m_mix_freq = gsl_data_handle_mix_freq (dhandle);

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
