// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplanwindow.hh"
#include "smstdioout.hh"

using namespace SpectMorph;
using std::string;

MorphPlanWindow::MorphPlanWindow (int width, int height, PuglNativeWindow win_id, bool resize, MorphPlanPtr morph_plan) :
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

  MenuItem *import_item = file_menu->add_item ("Import Preset...");
  connect (import_item->signal_clicked, this, &MorphPlanWindow::on_file_import_clicked);

  MenuItem *export_item = file_menu->add_item ("Export Preset...");
  connect (export_item->signal_clicked, this, &MorphPlanWindow::on_file_export_clicked);

  MenuItem *load_index_item = file_menu->add_item ("Load Index...");
  connect (load_index_item->signal_clicked, this, &MorphPlanWindow::on_load_index_clicked);

  fill_preset_menu (preset_menu);
  add_op_menu_item (op_menu, "Source", "SpectMorph::MorphSource");
  // add_op_menu_item (op_menu, "Output", "SpectMorph::MorphOutput"); <- we have only one output
  add_op_menu_item (op_menu, "Linear Morph", "SpectMorph::MorphLinear");
  add_op_menu_item (op_menu, "Grid Morph", "SpectMorph::MorphGrid");
  add_op_menu_item (op_menu, "LFO", "SpectMorph::MorphLFO");
  set_items (help_menu, {"About..."});
  grid.add_widget (menu_bar, 1, 1, 91, 3);

  ScrollView *scroll_view = new ScrollView (this);
  grid.add_widget (scroll_view, 1, 5, 47, height / 8 - 6);

  Widget *output_parent = new Widget (this);
  grid.add_widget (output_parent, 49, 5, 47, height / 8 - 6);

  MorphPlanView *mp_view = new MorphPlanView (scroll_view, output_parent, morph_plan.c_ptr(), this);
  auto update_mp_size = [=]() {
    scroll_view->set_scroll_widget (mp_view, false, true);
  };
  connect (mp_view->signal_widget_size_changed, update_mp_size);
  update_mp_size();
}

void
MorphPlanWindow::fill_preset_menu (Menu *menu)
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

void
MorphPlanWindow::add_op_menu_item (Menu *op_menu, const std::string& text, const std::string& type)
{
  MenuItem *item = op_menu->add_item (text);

  connect (item->signal_clicked, [=]()
    {
      MorphOperator *op = MorphOperator::create (type, m_morph_plan.c_ptr());

      g_return_if_fail (op != NULL);

      m_morph_plan->add_operator (op, MorphPlan::ADD_POS_AUTO);
    });
}


bool
MorphPlanWindow::load (const std::string& filename)
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
MorphPlanWindow::on_load_preset (const std::string& rel_filename)
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

void
MorphPlanWindow::on_file_import_clicked()
{
  open_file_dialog ("Select SpectMorph Preset to import", [=](string filename) {
    if (filename != "" && !load (filename))
      {
#if 0
        QMessageBox::critical (this, "Error",
                               string_locale_printf ("Import failed, unable to open file '%s'.", file_name_local.data()).c_str());
#endif
      }
  });
}

void
MorphPlanWindow::on_file_export_clicked()
{
  save_file_dialog ("Select SpectMorph Preset to export", [=](string filename) {
    GenericOut *out = StdioOut::open (filename);
    if (out)
      {
        m_morph_plan->save (out);
        delete out; // close file

        set_filename (filename);
      }
    else
      {
#if 0
        QMessageBox::critical (this, "Error",
                               string_locale_printf ("Export failed, unable to open file '%s'.", file_name_local.data()).c_str());
#endif
      }
  });
}

void
MorphPlanWindow::on_load_index_clicked()
{
  open_file_dialog ("Select SpectMorph Instrument Index", [=](string filename) {
    if (filename != "")
      {
        m_morph_plan->load_index (filename);
      }
  });
}
