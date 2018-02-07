// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_WINDOW_HH
#define SPECTMORPH_MORPH_PLAN_WINDOW_HH

#include "smwindow.hh"
#include "smlabel.hh"
#include <functional>

namespace SpectMorph
{

struct MorphPlanWindow : public Window
{
public:
  MorphPlanWindow (int width, int height, PuglNativeWindow win_id = 0, bool resize = true) :
    Window (width, height, win_id, resize)
  {
    Label *menu_label = new Label (this, "-- SpectMorph --");
  }
};

}

#endif
