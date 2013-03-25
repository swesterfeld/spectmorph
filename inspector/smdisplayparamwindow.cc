// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smdisplayparamwindow.hh"
#include <assert.h>
#include <birnet/birnet.hh>

#include <QVBoxLayout>

using namespace SpectMorph;

DisplayParamWindow::DisplayParamWindow()
{
  setWindowTitle ("Display Parameters");

  QVBoxLayout *vbox = new QVBoxLayout();

  show_lpc_checkbox = new QCheckBox ("Show LPC Envelope in Spectrum View");
  show_lsf_checkbox = new QCheckBox ("Show LPC LSF Parameters in Spectrum View");

  connect (show_lpc_checkbox, SIGNAL (clicked()), this, SLOT (on_param_changed()));
  connect (show_lsf_checkbox, SIGNAL (clicked()), this, SLOT (on_param_changed()));

  vbox->addWidget (show_lpc_checkbox);
  vbox->addWidget (show_lsf_checkbox);

  setLayout (vbox);
}

void
DisplayParamWindow::on_param_changed()
{
  Q_EMIT params_changed();
}

bool
DisplayParamWindow::show_lsf()
{
  return show_lsf_checkbox->isChecked();
}

bool
DisplayParamWindow::show_lpc()
{
  return show_lpc_checkbox->isChecked();
}
