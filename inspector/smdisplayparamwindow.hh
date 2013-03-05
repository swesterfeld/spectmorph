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


#ifndef SPECTMORPH_DISPLAY_PARAMWINDOW_HH
#define SPECTMORPH_DISPLAY_PARAMWINDOW_HH

#include "smspectrumview.hh"
#include "smtimefreqview.hh"
#include "smzoomcontroller.hh"

#include <QCheckBox>

namespace SpectMorph {

class DisplayParamWindow : public QWidget
{
  Q_OBJECT

  QCheckBox         *show_lsf_checkbox;
  QCheckBox         *show_lpc_checkbox;

public:
  DisplayParamWindow();

  bool show_lsf();
  bool show_lpc();

public slots:
  void on_param_changed();

signals:
  void params_changed();
};

}

#endif
