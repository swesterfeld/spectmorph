/*
 * Copyright (C) 2010-2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtkmm.h>
#include <assert.h>
#include <sys/time.h>

#include <vector>
#include <string>
#include <iostream>

#include <QMenuBar>
#include <QFileDialog>

#include "smmain.hh"
#include "smmorphplan.hh"
#include "smmorphsource.hh"
#include "smmorphoutput.hh"
#include "smmorphlinear.hh"
#include "smmorphplanview.hh"
#include "smmorphplanwindow.hh"
#include "smrenameoperatordialog.hh"
#include "smmorphoperatorview.hh"
#include "smstdioout.hh"
#include "smmemout.hh"
#include "smhexstring.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphPlanWindow::MorphPlanWindow (MorphPlanPtr morph_plan, const string& title) :
  m_morph_plan (morph_plan)
{
  /* actions ... */
  QAction *import_action = new QAction ("&Import...", this);
  QAction *export_action = new QAction ("&Export...", this);
  QAction *load_index_action = new QAction ("&Load Index...", this);

  connect (import_action, SIGNAL (triggered()), this, SLOT (on_file_import_clicked()));
  connect (export_action, SIGNAL (triggered()), this, SLOT (on_file_export_clicked()));
  connect (load_index_action, SIGNAL (triggered()), this, SLOT (on_load_index_clicked()));

  /* menus... */
  QMenuBar *menu_bar = menuBar();

  QMenu *file_menu = menu_bar->addMenu ("&File");
  file_menu->addAction (import_action);
  file_menu->addAction (export_action);
  file_menu->addAction (load_index_action);

  QMenu *edit_menu = menu_bar->addMenu ("&Edit");
}

void
MorphPlanWindow::on_file_import_clicked()
{
  QString file_name = QFileDialog::getOpenFileName (this, "Select SpectMorph plan to import", "", "SpectMorph plan files (*.smplan)");
  if (!file_name.isEmpty())
    {
      QByteArray file_name_local = QFile::encodeName (file_name);
      GenericIn *in = StdioIn::open (file_name_local.data());
      if (in)
        {
          m_morph_plan->load (in);
          delete in; // close file
        }
      else
        {
          QMessageBox::critical (this, "Error",
                                 Birnet::string_printf ("Import failed, unable to open file '%s'.", file_name_local.data()).c_str());
        }
    }
}

void
MorphPlanWindow::on_file_export_clicked()
{
  QString file_name = QFileDialog::getSaveFileName (this, "Select SpectMorph plan file", "", "SpectMorph plan files (*.smplan)");
  if (!file_name.isEmpty())
    {
      QByteArray file_name_local = QFile::encodeName (file_name);
      GenericOut *out = StdioOut::open (file_name_local.data());
      if (out)
        {
          m_morph_plan->save (out);
          delete out; // close file
        }
      else
        {
          QMessageBox::critical (this, "Error",
                                 Birnet::string_printf ("Export failed, unable to open file '%s'.", file_name_local.data()).c_str());
        }
    }
}

void
MorphPlanWindow::on_load_index_clicked()
{
  QFileDialog::getOpenFileName (this, "Select SpectMorph index file", "", "SpectMorph index files (*.smindex)");
#if 0
  Gtk::FileChooserDialog dialog ("Select SpectMorph index file", Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for (*this);

  // buttons
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  // allow only .smindex files
  Gtk::FileFilter filter_smindex;
  filter_smindex.set_name ("SpectMorph index files");
  filter_smindex.add_pattern ("*.smindex");
  dialog.add_filter (filter_smindex);

  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK)
    {
      m_morph_plan->load_index (dialog.get_filename());
    }
#endif
}


#if 0
void
MorphPlanWindow::on_load_index_clicked()
{
  Gtk::FileChooserDialog dialog ("Select SpectMorph index file", Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for (*this);

  // buttons
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  // allow only .smindex files
  Gtk::FileFilter filter_smindex;
  filter_smindex.set_name ("SpectMorph index files");
  filter_smindex.add_pattern ("*.smindex");
  dialog.add_filter (filter_smindex);

  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK)
    {
      m_morph_plan->load_index (dialog.get_filename());
    }
}

MorphPlanWindow::MorphPlanWindow (MorphPlanPtr morph_plan, const string& title) :
  m_morph_plan (morph_plan),
  morph_plan_view (morph_plan.c_ptr(), this)
{
  set_default_size (250, 100);
  set_title (title);
  plan_vbox.set_border_width (10);

  ref_action_group = Gtk::ActionGroup::create();
  ref_action_group->add (Gtk::Action::create ("FileMenu", "File"));
  ref_action_group->add (Gtk::Action::create ("FileImport", "Import..."),
                         sigc::mem_fun (*this, &MorphPlanWindow::on_file_import_clicked));
  ref_action_group->add (Gtk::Action::create ("FileExport", "Export..."),
                         sigc::mem_fun (*this, &MorphPlanWindow::on_file_export_clicked));
  ref_action_group->add (Gtk::Action::create ("EditMenu", "Edit"));
  ref_action_group->add (Gtk::Action::create ("EditAddOperator", "Add Operator"));

  // Source
  ref_action_group->add (Gtk::Action::create ("EditAddSource", "Source"),
                         sigc::bind (sigc::mem_fun (*this, &MorphPlanWindow::add_operator),
                                     "SpectMorph::MorphSource"));
  // Output
  ref_action_group->add (Gtk::Action::create ("EditAddOutput", "Output"),
                         sigc::bind (sigc::mem_fun (*this, &MorphPlanWindow::add_operator),
                                     "SpectMorph::MorphOutput"));
  // Linear
  ref_action_group->add (Gtk::Action::create ("EditAddLinearMorph", "Linear Morph"),
                         sigc::bind (sigc::mem_fun (*this, &MorphPlanWindow::add_operator),
                                     "SpectMorph::MorphLinear"));
  // LFO
  ref_action_group->add (Gtk::Action::create ("EditAddLFO", "LFO"),
                         sigc::bind (sigc::mem_fun (*this, &MorphPlanWindow::add_operator),
                                     "SpectMorph::MorphLFO"));



  ref_action_group->add (Gtk::Action::create ("FileLoadIndex", "Load Index"),
                         sigc::mem_fun (*this, &MorphPlanWindow::on_load_index_clicked));

  ref_action_group->add (Gtk::Action::create ("ContextMenu", "Context Menu"));

  ref_action_group->add (Gtk::Action::create ("ContextRename", "Rename"),
          sigc::mem_fun(*this, &MorphPlanWindow::on_context_rename));

  ref_action_group->add(Gtk::Action::create ("ContextRemove", "Remove"),
          //Gtk::AccelKey("<control>P"),
          sigc::mem_fun (*this, &MorphPlanWindow::on_context_remove));


  ref_ui_manager = Gtk::UIManager::create();
  ref_ui_manager-> insert_action_group (ref_action_group);
  add_accel_group (ref_ui_manager->get_accel_group());

  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='FileImport' />"
    "      <menuitem action='FileExport' />"
    "      <menuitem action='FileLoadIndex' />"
    "    </menu>"
    "    <menu action='EditMenu'>"
    "      <menu action='EditAddOperator'>"
    "        <menuitem action='EditAddSource' />"
    "        <menuitem action='EditAddOutput' />"
    "        <menuitem action='EditAddLinearMorph' />"
    "        <menuitem action='EditAddLFO' />"
    "      </menu>"
    "    </menu>"
    "  </menubar>"
    "  <popup name='PopupMenu'>"
    "    <menuitem action='ContextRename'/>"
    "    <menuitem action='ContextRemove'/>"
    "  </popup>"
    "</ui>";
  try
    {
      ref_ui_manager->add_ui_from_string (ui_info);
    }
  catch (const Glib::Error& ex)
    {
      std::cerr << "building menus failed: " << ex.what();
    }

  popup_menu = dynamic_cast<Gtk::Menu*>(ref_ui_manager->get_widget ("/PopupMenu"));
  if(!popup_menu)
    g_warning ("popup menu not found");

  Gtk::Widget *menu_bar = ref_ui_manager->get_widget ("/MenuBar");
  if (menu_bar)
    window_vbox.pack_start (*menu_bar, Gtk::PACK_SHRINK);
  window_vbox.add (plan_vbox);

  plan_vbox.add (morph_plan_view);
  add (window_vbox);

  show_all_children();
}

void
MorphPlanWindow::on_context_rename()
{
  RenameOperatorDialog dialog (popup_op);

  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK)
    {
      popup_op->set_name (dialog.new_name());
    }
}

void
MorphPlanWindow::on_context_remove()
{
  m_morph_plan->remove (popup_op);
}

void
MorphPlanWindow::show_popup (GdkEventButton *event, MorphOperator *op)
{
  popup_op = op;
  popup_menu->popup (event->button, event->time);
}

MorphOperator *
MorphPlanWindow::where (MorphOperator *op, double x, double y)
{
  vector<int> start_position;
  int end_y = 0;

  MorphOperator *result = NULL;

  const vector<MorphOperatorView *> op_views = morph_plan_view.op_views();
  if (!op_views.empty())
    result = op_views[0]->op();

  for (vector<MorphOperatorView *>::const_iterator vi = op_views.begin(); vi != op_views.end(); vi++)
    {
      MorphOperatorView *view = *vi;

      int view_x, view_y;
      view->get_window()->get_origin (view_x, view_y);

      end_y = view_y + view->get_height();
      if (view_y < y)
        result = view->op();
    }

  if (y > end_y)
    return 0;
  else
    return result;
}

void
MorphPlanWindow::on_file_import_clicked()
{
  Gtk::FileChooserDialog dialog ("Select SpectMorph plan file to import", Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for (*this);

  // buttons
  dialog.add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button (Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

  // allow only .smplan files
  Gtk::FileFilter filter_smplan;
  filter_smplan.set_name ("SpectMorph plan files");
  filter_smplan.add_pattern ("*.smplan");
  dialog.add_filter (filter_smplan);

  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK)
    {
      GenericIn *in = StdioIn::open (dialog.get_filename());
      if (in)
        {
          m_morph_plan->load (in);
          delete in; // close file
        }
      else
        {
          Gtk::MessageDialog dlg (Birnet::string_printf ("Import failed, unable to open file '%s'.",
                                                         dialog.get_filename().c_str()), false, Gtk::MESSAGE_ERROR);
          dlg.run();
        }
    }
}

void
MorphPlanWindow::on_file_export_clicked()
{
  Gtk::FileChooserDialog dialog ("Select SpectMorph plan file", Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.set_transient_for (*this);

  // buttons
  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

  // allow only .smplan files
  Gtk::FileFilter filter_smplan;
  filter_smplan.set_name ("SpectMorph plan files");
  filter_smplan.add_pattern ("*.smplan");
  dialog.add_filter (filter_smplan);

  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK)
    {
      GenericOut *out = StdioOut::open (dialog.get_filename());
      if (out)
        {
          m_morph_plan->save (out);
          delete out; // close file
        }
      else
        {
          Gtk::MessageDialog dlg (Birnet::string_printf ("Export failed, unable to open file '%s'.",
                                                         dialog.get_filename().c_str()), false, Gtk::MESSAGE_ERROR);
          dlg.run();
        }
    }
}

void
MorphPlanWindow::add_operator (const string& type)
{
  MorphOperator *op = MorphOperator::create (type, m_morph_plan.c_ptr());

  g_return_if_fail (op != NULL);

  m_morph_plan->add_operator (op);
}
#endif
