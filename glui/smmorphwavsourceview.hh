// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_WAV_SOURCE_VIEW_HH
#define SPECTMORPH_MORPH_WAV_SOURCE_VIEW_HH

#include "smmorphoperatorview.hh"
#include "smmorphwavsource.hh"
#include "smmorphplanwindow.hh"

namespace SpectMorph
{

class MorphWavSourceView : public MorphOperatorView
{
  MorphWavSource   *morph_wav_source;

  void on_load();
  void on_edit();
public:
  MorphWavSourceView (Widget *parent, MorphWavSource *morph_wav_source, MorphPlanWindow *morph_plan_window);

  double view_height() override;
};

}

#endif
