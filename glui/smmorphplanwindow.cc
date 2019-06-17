// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphplanwindow.hh"
#include "smstdioout.hh"
#include "smaboutdialog.hh"
#include "smmessagebox.hh"
#include "smeventloop.hh"
#include "smconfig.hh"

using namespace SpectMorph;
using std::string;
using std::vector;

// unfortunately some hosts need to know the window size before creating the window
// so we provide a way to compute it without creating a window
namespace
{
  static const int win_width = 744;
  static const int win_height = 560;
};

void
MorphPlanWindow::static_scaled_size (int *w, int *h)
{
  const Config cfg;
  const double global_scale = cfg.zoom() / 100.0;

  *w = win_width * global_scale;
  *h = win_height * global_scale;
}

MorphPlanWindow::MorphPlanWindow (EventLoop& event_loop,
                                  const string& title, PuglNativeWindow win_id, bool resize, MorphPlanPtr morph_plan) :
  Window (event_loop, title, win_width, win_height, win_id, resize),
  m_morph_plan (morph_plan),
  m_synth_interface (morph_plan->project()->synth_interface())
{
  FixedGrid grid;

  MenuBar *menu_bar = new MenuBar (this);
  Menu *zoom_menu = menu_bar->add_menu ("Zoom");
  Menu *file_menu = menu_bar->add_menu ("File");
  Menu *preset_menu = menu_bar->add_menu ("Open Preset");
  Menu *op_menu = menu_bar->add_menu ("Add Operator");
  Menu *help_menu = menu_bar->add_menu ("Help");

  fill_zoom_menu (zoom_menu);
  MenuItem *import_item = file_menu->add_item ("Import Preset...");
  connect (import_item->signal_clicked, this, &MorphPlanWindow::on_file_import_clicked);

  MenuItem *export_item = file_menu->add_item ("Export Preset...");
  connect (export_item->signal_clicked, this, &MorphPlanWindow::on_file_export_clicked);

  fill_preset_menu (preset_menu);
  add_op_menu_item (op_menu, "Source", "SpectMorph::MorphSource");
  add_op_menu_item (op_menu, "Wav Source", "SpectMorph::MorphWavSource");
  // add_op_menu_item (op_menu, "Output", "SpectMorph::MorphOutput"); <- we have only one output
  add_op_menu_item (op_menu, "Linear Morph", "SpectMorph::MorphLinear");
  add_op_menu_item (op_menu, "Grid Morph", "SpectMorph::MorphGrid");
  add_op_menu_item (op_menu, "LFO", "SpectMorph::MorphLFO");

  MenuItem *about_item = help_menu->add_item ("About...");
  connect (about_item->signal_clicked, this, &MorphPlanWindow::on_about_clicked);

  grid.add_widget (menu_bar, 1, 1, 91, 3);

  ScrollView *scroll_view = new ScrollView (this);
  grid.add_widget (scroll_view, 1, 5, 47, height / 8 - 6);

  Widget *output_parent = new Widget (this);
  grid.add_widget (output_parent, 49, 5, 47, height / 8 - 6);

  m_morph_plan_view = new MorphPlanView (scroll_view, output_parent, morph_plan.c_ptr(), this);
  scroll_view->set_scroll_widget (m_morph_plan_view, false, true);

  connect (m_morph_plan_view->signal_widget_size_changed, scroll_view, &ScrollView::on_widget_size_changed);

  /* control widget */
  m_control_widget = new MorphPlanControl (this, m_morph_plan);
  double cw_height = m_control_widget->view_height();
  grid.add_widget (m_control_widget, 49, height / 8 - cw_height - 1, 43, cw_height);
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


Error
MorphPlanWindow::load (const std::string& filename)
{
  Error error = m_morph_plan->project()->load (filename);

  if (!error)
    set_filename (filename);

  return error;
}

Error
MorphPlanWindow::save (const std::string& filename)
{
  Error error = m_morph_plan->project()->save (filename);

  if (!error)
    set_filename (filename);

  return error;
}

void
MorphPlanWindow::on_load_preset (const std::string& rel_filename)
{
  std::string filename = sm_get_install_dir (INSTALL_DIR_TEMPLATES) + "/" + rel_filename;

  Error error = load (filename);
  if (error)
    {
        MessageBox::critical (this, "Error",
                              string_locale_printf ("Loading preset failed, unable to open file:\n'%s'\n%s.", filename.c_str(), error.message()));
    }
}

void
MorphPlanWindow::on_file_import_clicked()
{
  FileDialogFormats formats ("SpectMorph Preset files", "smplan");
  open_file_dialog ("Select SpectMorph Preset to import", formats, [=](string filename) {
    if (filename != "")
      {
        Error error = load (filename);

        if (error)
          {
            MessageBox::critical (this, "Error",
                                  string_locale_printf ("Import failed, unable to open file:\n'%s'\n%s.", filename.c_str(), error.message()));
          }
      }
  });
}

void
MorphPlanWindow::on_file_export_clicked()
{
  FileDialogFormats formats ("SpectMorph Preset files", "smplan");
  save_file_dialog ("Select SpectMorph Preset export filename", formats, [=](string filename) {
    if (filename != "")
      {
        Error error = save (filename);

        if (error)
          {
            MessageBox::critical (this, "Error",
                                  string_locale_printf ("Export failed, unable to save to file:\n'%s'.\n%s", filename.c_str(), error.message()));
          }
      }
  });
}

MorphOperator *
MorphPlanWindow::where (MorphOperator *op, double y)
{
  MorphOperator *result = nullptr;

  double end_y = 0;

  const vector<MorphOperatorView *> op_views = m_morph_plan_view->op_views();
  if (!op_views.empty())
    result = op_views[0]->op();

  for (auto view : op_views)
    {
      if (!view->is_output())
        {
          if (view->abs_y() < y)
            result = view->op();

          end_y = view->abs_y() + view->height;
        }
    }

  if (y > end_y)  // below last operator?
    return nullptr;
  else
    return result;
}

void
MorphPlanWindow::on_about_clicked()
{
  auto dialog = new AboutDialog (this);

  dialog->run();
}

MorphPlanControl *
MorphPlanWindow::control_widget()
{
  return m_control_widget;
}

SynthInterface*
MorphPlanWindow::synth_interface()
{
  return m_synth_interface;
}
