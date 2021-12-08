// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html
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
  set_x ((window()->width() - width()) / 2);
  set_y ((window()->height() - height()) / 2);

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
