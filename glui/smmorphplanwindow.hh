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
#include "smmorphplancontrol.hh"
#include <functional>

namespace SpectMorph
{

class MorphPlanView;
class MorphPlanWindow : public Window
{
  MorphPlanPtr      m_morph_plan;
  MorphPlanView    *m_morph_plan_view = nullptr;
  MorphPlanControl *m_control_widget = nullptr;
  std::string       m_filename;

  void add_op_menu_item (Menu *op_menu, const std::string& text, const std::string& op_name);
public:
  void
  set_filename (const std::string& filename)
  {
    m_filename = filename;
    // FIXME update_window_title();
  }

  MorphPlanWindow (const std::string& title, PuglNativeWindow win_id, bool resize, MorphPlanPtr morph_plan,
                   MorphPlanControl::Features f = MorphPlanControl::ALL_WIDGETS);

  void fill_preset_menu (Menu *menu);
  void fill_zoom_menu (Menu *menu);
  bool load (const std::string& filename);
  void on_load_preset (const std::string& rel_filename);

  MorphPlanControl *control_widget();

  MorphOperator *where (MorphOperator *op, double y);

/* slots: */
  void on_file_import_clicked();
  void on_file_export_clicked();
  void on_load_index_clicked();
  void on_about_clicked();
};

}

#endif
