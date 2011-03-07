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

#include "smmain.hh"
#include "smmorphplan.hh"
#include "smmorphsource.hh"
#include "smmorphoutput.hh"
#include "smmorphlinear.hh"
#include "smmorphplanview.hh"
#include "smmainwindow.hh"

using namespace SpectMorph;

using std::string;

void
MainWindow::on_load_index_clicked()
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
      morph_plan.load_index (dialog.get_filename());
    }
}

void
MainWindow::on_add_source_clicked()
{
  morph_plan.add_operator (new MorphSource (&morph_plan));
}

void
MainWindow::on_add_output_clicked()
{
  morph_plan.add_operator (new MorphOutput (&morph_plan));
}

void
MainWindow::on_add_linear_morph_clicked()
{
  morph_plan.add_operator (new MorphLinear (&morph_plan));
}

MainWindow::MainWindow() :
  morph_plan_view (&morph_plan, this)
{
  plan_vbox.set_border_width (10);

  ref_action_group = Gtk::ActionGroup::create();
  ref_action_group->add (Gtk::Action::create ("EditMenu", "Edit"));
  ref_action_group->add (Gtk::Action::create ("EditAddOperator", "Add Operator"));
  ref_action_group->add (Gtk::Action::create ("EditAddSource", "Source"),
                         sigc::mem_fun (*this, &MainWindow::on_add_source_clicked));
  ref_action_group->add (Gtk::Action::create ("EditAddOutput", "Output"),
                         sigc::mem_fun (*this, &MainWindow::on_add_output_clicked));
  ref_action_group->add (Gtk::Action::create ("EditAddLinearMorph", "Linear Morph"),
                         sigc::mem_fun (*this, &MainWindow::on_add_linear_morph_clicked));
  ref_action_group->add (Gtk::Action::create ("EditLoadIndex", "Load Index"),
                         sigc::mem_fun (*this, &MainWindow::on_load_index_clicked));

  ref_action_group->add (Gtk::Action::create ("ContextMenu", "Context Menu"));

  ref_action_group->add (Gtk::Action::create ("ContextRename", "Rename"),
          sigc::mem_fun(*this, &MainWindow::on_context_rename));

  ref_action_group->add(Gtk::Action::create("ContextRemove", "Remove"),
          //Gtk::AccelKey("<control>P"),
          sigc::mem_fun(*this, &MainWindow::on_context_remove));


  ref_ui_manager = Gtk::UIManager::create();
  ref_ui_manager-> insert_action_group (ref_action_group);
  add_accel_group (ref_ui_manager->get_accel_group());

  Glib::ustring ui_info =
    "<ui>"
    "  <menubar name='MenuBar'>"
    "    <menu action='EditMenu'>"
    "      <menu action='EditAddOperator'>"
    "        <menuitem action='EditAddSource' />"
    "        <menuitem action='EditAddOutput' />"
    "        <menuitem action='EditAddLinearMorph' />"
    "      </menu>"
    "      <menuitem action='EditLoadIndex' />"
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
MainWindow::set_plan_str (const string& plan_str)
{
  morph_plan.set_plan_str (plan_str);
}

void
MainWindow::on_context_rename()
{
  // implement me
}

void
MainWindow::on_context_remove()
{
  morph_plan.remove (popup_op);
}

void
MainWindow::show_popup (GdkEventButton *event, MorphOperator *op)
{
  popup_op = op;
  popup_menu->popup (event->button, event->time);
}
