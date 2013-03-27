// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_WAVSET_REPO_HH
#define SPECTMORPH_WAVSET_REPO_HH

#include "smwavset.hh"

#include <bse/bsecxxplugin.hh>

#include <QMutex>

#include <map>

namespace SpectMorph
{

class WavSetRepo {
  QMutex mutex;
  std::map<std::string, WavSet *> wav_set_map;
public:
  WavSet *get (const std::string& filename);

  static WavSetRepo *the(); // Singleton
};

}

#endif
