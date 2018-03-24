// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smwavsetrepo.hh"

using namespace SpectMorph;

using std::string;

WavSetRepo*
WavSetRepo::the()
{
  static WavSetRepo *instance = NULL;
  if (!instance)
    instance = new WavSetRepo;

  return instance;
}

WavSet*
WavSetRepo::get (const string& filename)
{
  std::lock_guard<std::mutex> lock (mutex);

  WavSet*& wav_set = wav_set_map[filename];
  if (!wav_set)
    {
      wav_set = new WavSet();
      wav_set->load (filename, AUDIO_SKIP_DEBUG);
    }
  return wav_set;
}

void
WavSetRepo::cleanup()
{
  /* cleanup wave repo: this should only be called at program termination */
  for (auto w : WavSetRepo::the()->wav_set_map)
    delete w.second;
}
