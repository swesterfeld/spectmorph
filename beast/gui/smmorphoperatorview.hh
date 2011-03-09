/*
 * Copyright (C) 2011 Stefan Westerfeld
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

#ifndef SPECTMORPH_MORPH_OPERATOR_VIEW_HH
#define SPECTMORPH_MORPH_OPERATOR_VIEW_HH

#include <gtkmm.h>
#include "smmainwindow.hh"

namespace SpectMorph
{

class MorphOperator;
class MorphOperatorView : public Gtk::EventBox
{
protected:
  Gtk::Frame     frame;
  MainWindow    *main_window;
  MorphOperator *op;

  void on_operators_changed();
public:
  MorphOperatorView (MorphOperator *op, MainWindow *main_window);

  bool on_button_press_event (GdkEventButton *event);
};

}

#endif
