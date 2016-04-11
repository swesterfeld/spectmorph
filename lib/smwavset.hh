// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WAVSET_HH
#define SPECTMORPH_WAVSET_HH

#include <vector>
#include <string>
#include <bse/bsecxxplugin.hh>

#include "smaudio.hh"

namespace SpectMorph
{

class WavSetWave
{
public:
  int         midi_note;
  int         channel;
  int         velocity_range_min;
  int         velocity_range_max;
  std::string path;
  Audio      *audio;

  WavSetWave();
  ~WavSetWave();
};

class WavSet
{
public:
  ~WavSet();

  std::string              name;
  std::string              short_name;
  std::vector<WavSetWave>  waves;

  void clear();

  Bse::Error load (const std::string& filename, AudioLoadOptions load_options = AUDIO_LOAD_DEBUG);
  Bse::Error save (const std::string& filename, bool embed_models = false);
};

}

#endif
