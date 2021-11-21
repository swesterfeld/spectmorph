// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_ABOUT_DIALOG_HH
#define SPECTMORPH_ABOUT_DIALOG_HH

#include "smdialog.hh"

namespace SpectMorph
{

class AboutDialog : public Dialog
{
public:
  AboutDialog (Window *window);

  void
  key_press_event (const PuglEventKey& key_event) override
  {
    on_accept();
  }
  void
  mouse_press (const MouseEvent& event) override
  {
    on_accept();
  }
};

}

#endif
