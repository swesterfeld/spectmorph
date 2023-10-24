// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smdialog.hh"
#include "smbutton.hh"

namespace SpectMorph
{

class VolumeResetDialog : public Dialog
{
public:
  enum class Result {
    LOOP_ENERGY,
    ZERO,
    REMOVE_AVERAGE
  };
  Signal<Result> signal_result;
  VolumeResetDialog (Window *window) :
    Dialog (window)
  {
    FixedGrid grid;

    double yoffset;

    double tw = 0;
    auto mk_label = [&] (const std::string& s)
      {
        auto label = new Label (this, s);
        tw = std::max (tw, DrawUtils::static_text_width (window, s));
        grid.add_widget (label, 2, yoffset, tw / 8 + 1, 3);
        return label;
      };
    yoffset = 3;
    mk_label ("Reset Volumes")->set_bold (true);
    yoffset += 4;
    mk_label ("This operation will assign new values to all volumes.");


    double w = tw / 8 + 25;
    yoffset = 2;

    auto button1 = new Button (this, "Normalize Loop Energy");
    grid.add_widget (button1, w - 19, yoffset, 18, 3);
    connect (button1->signal_clicked, [&](){ signal_result (Result::LOOP_ENERGY); on_accept();});
    yoffset += 3;

    auto button2 = new Button (this, "Reset all volumes to zero");
    grid.add_widget (button2, w - 19, yoffset, 18, 3);
    connect (button2->signal_clicked, [&](){ signal_result (Result::ZERO); on_accept();});
    yoffset += 3;

    auto button3 = new Button (this, "Remove average value");
    grid.add_widget (button3, w - 19, yoffset, 18, 3);
    connect (button3->signal_clicked, [&](){ signal_result (Result::REMOVE_AVERAGE); on_accept();});
    yoffset += 3;

    auto cancel_button = new Button (this, "Cancel");
    grid.add_widget (cancel_button, w - 19, yoffset, 18, 3);
    connect (cancel_button->signal_clicked, [&](){ on_reject();});
    yoffset += 3;

    grid.add_widget (new VLine (this, Color (0.6, 0.6, 0.6), 2), tw / 8 + 4, 1, 1, yoffset);

    window->set_keyboard_focus (this);

    grid.add_widget (this, 0, 0, w, yoffset + 2);
  }
};

}
