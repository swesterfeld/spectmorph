// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smwavsetrepo.hh"
#include "smmain.hh"

#include <memory>

using namespace SpectMorph;

using std::string;

WavSet*
WavSetRepo::get (const string& filename)
{
  std::lock_guard<std::mutex> lock (mutex);

  auto cached = wav_set_map.find (filename);
  if (cached != wav_set_map.end())
    return cached->second;

  auto wav_set = std::make_unique<WavSet>();
  if (wav_set->load (filename, AUDIO_SKIP_DEBUG))
    wav_set.reset();

  auto result = wav_set.get();
  wav_set_map.emplace (filename, result);
  wav_set.release();
  return result;
}

WavSetRepo::~WavSetRepo()
{
  for (auto w : wav_set_map)
    delete w.second;
}
