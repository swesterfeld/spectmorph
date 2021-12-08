// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smwavsetrepo.hh"
#include "smmain.hh"

using namespace SpectMorph;

using std::string;

WavSetRepo*
WavSetRepo::the()
{
  return Global::wav_set_repo();
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

WavSetRepo::~WavSetRepo()
{
  for (auto w : wav_set_map)
    delete w.second;
}
