// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_DIALOG_HH
#define SPECTMORPH_DIALOG_HH

#include "smwindow.hh"
#include "smframe.hh"

namespace SpectMorph
{

class Dialog : public Frame
{
  std::function<void (bool)> m_done_callback;

public:
  Dialog (Window *parent);

  void run (std::function<void (bool)> done_callback = nullptr);

  void on_accept();
  void on_reject();
};

}

#endif
