// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
