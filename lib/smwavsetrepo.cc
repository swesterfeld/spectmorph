// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include <bse/bseloader.h>
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
  Birnet::AutoLocker lock (mutex);

  WavSet*& wav_set = wav_set_map[filename];
  if (!wav_set)
    {
      wav_set = new WavSet();
      wav_set->load (filename, AUDIO_SKIP_DEBUG);
    }
  return wav_set;
}
