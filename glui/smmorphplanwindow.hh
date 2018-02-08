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

struct MorphPlanWindow : public Window
{
  MorphPlanPtr m_morph_plan;
  std::string  m_filename;

  void
  set_filename (const std::string& filename)
  {
    m_filename = filename;
    // FIXME update_window_title();
  }
public:
  MorphPlanWindow (int width, int height, PuglNativeWindow win_id, bool resize, MorphPlanPtr morph_plan) :
    Window (width, height, win_id, resize),
    m_morph_plan (morph_plan)
  {
    FixedGrid grid;

    MenuBar *menu_bar = new MenuBar (this);
    Menu *file_menu = menu_bar->add_menu ("File");
    Menu *preset_menu = menu_bar->add_menu ("Open Preset");
    Menu *op_menu = menu_bar->add_menu ("Add Operator");
    Menu *help_menu = menu_bar->add_menu ("Help");

    auto set_items = [this](Menu *m, const std::vector<std::string>& items) {
      for (auto i : items) {
        MenuItem *item = m->add_item (i);
        connect (item->signal_clicked, [=]()
          {
            printf ("menu item %s selected\n", i.c_str());
          });
      }
    };
    set_items (file_menu, {"Import...", "Export...", "Load Instrument Set..."});
    fill_preset_menu (preset_menu);
    set_items (op_menu, {"Source", "Output", "Linear Morph", "Grid Morph", "LFO" });
    set_items (help_menu, {"About..."});
    grid.add_widget (menu_bar, 1, 1, 46, 3);

    grid.add_widget (new MorphPlanView (this, morph_plan.c_ptr()), 1, 5, 44, 20);
    grid.add_widget (new ScrollBar (this, 0.3), 45, 5, 2, 42);
  }
  void
  fill_preset_menu (Menu *menu)
  {
    MicroConf cfg (sm_get_install_dir (INSTALL_DIR_TEMPLATES) + "/index.smpindex");

    if (!cfg.open_ok())
      {
        return;
      }

    while (cfg.next())
      {
        std::string filename, title;

        if (cfg.command ("plan", filename, title))
          {
            MenuItem *item = menu->add_item (title);
            connect (item->signal_clicked, [=]()
              {
                on_load_preset (filename);
              });
          }
      }
  }

  bool
  load (const std::string& filename)
  {
    GenericIn *in = StdioIn::open (filename);
    if (in)
      {
        m_morph_plan->load (in);
        delete in; // close file

        set_filename (filename);

        return true;
      }
    return false;
  }

  void
  on_load_preset (const std::string& rel_filename)
  {
    std::string filename = sm_get_install_dir (INSTALL_DIR_TEMPLATES) + "/" + rel_filename;

    if (!load (filename))
      {
#if 0 // FIXME
        QMessageBox::critical (this, "Error",
                               string_locale_printf ("Loading template failed, unable to open file '%s'.", filename.c_str()).c_str());
#endif
      }
  }

};

}

#endif
