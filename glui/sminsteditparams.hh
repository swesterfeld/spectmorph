// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INST_EDIT_PARAMS_HH
#define SPECTMORPH_INST_EDIT_PARAMS_HH

#include "smcheckbox.hh"

namespace SpectMorph
{

class InstEditParams : public Window
{
public:
  InstEditParams (Window *window) :
    Window ("Instrument Params", 320, 88, 0, false, window->native_window())
  {
    window->add_child_window (this);
    set_close_callback ([this,window]() { window->remove_child_window (this); });

    FixedGrid grid;
    auto auto_volume_checkbox = new CheckBox (this, "Auto Volume");
    connect (auto_volume_checkbox->signal_toggled, [](bool b) { printf ("foo\n"); });
    grid.add_widget (auto_volume_checkbox, 2, 2, 20, 2);

    show();
  }
};

}

#endif
