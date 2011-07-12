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

#ifndef SPECTMORPH_MORPH_PLAN_WINDOW_HH
#define SPECTMORPH_MORPH_PLAN_WINDOW_HH

#include <gtkmm.h>
#include <assert.h>
#include <sys/time.h>

#include <vector>
#include <string>

#include "smmain.hh"
#include "smmorphplan.hh"
#include "smmorphsource.hh"
#include "smmorphplanview.hh"
#include "smmorphoperator.hh"

namespace SpectMorph
{

class MorphPlanWindow : public Gtk::Window
{
  Gtk::VBox     window_vbox;
  Gtk::VBox     plan_vbox;
  MorphPlanPtr  m_morph_plan;
  MorphPlanView morph_plan_view;

  Glib::RefPtr<Gtk::UIManager>    ref_ui_manager;
  Glib::RefPtr<Gtk::ActionGroup>  ref_action_group;

  Gtk::Menu                      *popup_menu;
  MorphOperator                  *popup_op;
public:
  MorphPlanWindow (MorphPlanPtr morph_plan, const std::string& title);

  MorphPlanPtr morph_plan();

  void on_load_index_clicked();
  void on_file_import_clicked();
  void on_file_export_clicked();

  void on_context_rename();
  void on_context_remove();

  MorphOperator *where (MorphOperator *op, double x, double y);

  void show_popup (GdkEventButton *event, MorphOperator *op);
  void add_operator (const std::string& type);
};

}

#endif
