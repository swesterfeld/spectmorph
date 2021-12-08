// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
#include "smsynthinterface.hh"
#include <functional>

namespace SpectMorph
{

class MorphPlanView;
class MorphPlanWindow : public Window
{
  MorphPlanPtr      m_morph_plan;
  MorphPlanView    *m_morph_plan_view = nullptr;
  MorphPlanControl *m_control_widget = nullptr;
  SynthInterface   *m_synth_interface = nullptr;
  std::string       m_filename;

  void add_op_menu_item (Menu *op_menu, const std::string& text, const std::string& op_name);

  Error load (const std::string& filename);
  Error save (const std::string& filename);
public:
  void
  set_filename (const std::string& filename)
  {
    m_filename = filename;
    // FIXME update_window_title();
  }

  MorphPlanWindow (EventLoop& event_loop, const std::string& title, PuglNativeWindow win_id, bool resize, MorphPlanPtr morph_plan);

  void fill_preset_menu (Menu *menu);
  void on_load_preset (const std::string& rel_filename);
  static void static_scaled_size (int *w, int *h);

  SynthInterface   *synth_interface();

  MorphOperator *where (MorphOperator *op, double y);

/* slots: */
  void on_file_import_clicked();
  void on_file_export_clicked();
  void on_about_clicked();
};

}

#endif
