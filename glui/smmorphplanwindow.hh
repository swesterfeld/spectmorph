// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_WINDOW_HH
#define SPECTMORPH_MORPH_PLAN_WINDOW_HH

#include "smwindow.hh"
#include "smlabel.hh"
#include "smmorphplan.hh"
#include "smmorphplanview.hh"
#include "smmenubar.hh"
#include <functional>

namespace SpectMorph
{

struct MorphPlanWindow : public Window
{
public:
  MorphPlanWindow (int width, int height, PuglNativeWindow win_id, bool resize, MorphPlanPtr morph_plan) :
    Window (width, height, win_id, resize)
  {
    FixedGrid grid;

    MenuBar *menu_bar = new MenuBar (this);
    Menu *file_menu = menu_bar->add_menu ("File");
    Menu *preset_menu = menu_bar->add_menu ("Open Preset");
    Menu *op_menu = menu_bar->add_menu ("Add Operator");
    Menu *help_menu = menu_bar->add_menu ("Help");

    auto set_items = [](Menu *m, const std::vector<std::string>& items) {
      for (auto i : items) {
        MenuItem *item = m->add_item (i);
        item->set_callback ([=]()
          {
            printf ("menu item %s selected\n", i.c_str());
          });
      }
    };
    set_items (file_menu, {"Import...", "Export...", "Load Instrument Set..."});
    set_items (preset_menu, {"2x2 Grid Morph using GUI", "Fancy Preset", "Cool Preset" });
    set_items (op_menu, {"Source", "Output", "Linear Morph", "Grid Morph", "LFO" });
    set_items (help_menu, {"About..."});
    grid.add_widget (menu_bar, 1, 1, 46, 3);

    grid.add_widget (new MorphPlanView (this, morph_plan.c_ptr()), 1, 5, 44, 20);
  }
};

}

#endif
