// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_WINDOW_HH
#define SPECTMORPH_MORPH_PLAN_WINDOW_HH

#include "smwindow.hh"
#include "smlabel.hh"
#include "smmorphplan.hh"
#include "smmorphplanview.hh"
#include "smmenubar.hh"
#include "smscrollbar.hh"
#include "smmicroconf.hh"
#include <functional>

namespace SpectMorph
{

class MorphPlanWindow : public Window
{
  MorphPlanPtr m_morph_plan;
  std::string  m_filename;

  void
  set_filename (const std::string& filename)
  {
    m_filename = filename;
    // FIXME update_window_title();
  }
  void add_op_menu_item (Menu *op_menu, const std::string& text, const std::string& op_name);
public:
  MorphPlanWindow (int width, int height, PuglNativeWindow win_id, bool resize, MorphPlanPtr morph_plan);

  void fill_preset_menu (Menu *menu);
  bool load (const std::string& filename);
  void on_load_preset (const std::string& rel_filename);
};

}

#endif
