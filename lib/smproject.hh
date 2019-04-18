// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROJECT_HH
#define SPECTMORPH_PROJECT_HH

#include "sminstrument.hh"
#include "smwavset.hh"
#include "smwavsetbuilder.hh"

#include <thread>

namespace SpectMorph
{

class Project
{
  std::shared_ptr<WavSet> wav_set = nullptr;
public:
  Instrument instrument;

  void
  rebuild()
  {
    WavSetBuilder *builder = new WavSetBuilder (&instrument, /* keep_samples */ false);
    new std::thread ([this, builder]() {
      /* FIXME: sharing a pointer between threads can crash */
      wav_set = std::shared_ptr<WavSet> (builder->run());
      delete builder;
    });
  }
  std::shared_ptr<WavSet>
  get_wav_set()
  {
    return wav_set;
  }
};

}

#endif
