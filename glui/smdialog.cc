// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#include "smdialog.hh"

using namespace SpectMorph;

Dialog::Dialog (Window *window) :
  Frame (window)
{
}

void
Dialog::run (std::function<void (bool)> done_callback)
{
  /* center dialog on screen */
  x = (window()->width - width) / 2;
  y = (window()->height - height) / 2;

  window()->set_dialog_widget (this);

  m_done_callback = done_callback;
}

void
Dialog::on_accept()
{
  if (m_done_callback)
    m_done_callback (true);

  delete this;
}

void
Dialog::on_reject()
{
  if (m_done_callback)
    m_done_callback (false);

  delete this;
}
