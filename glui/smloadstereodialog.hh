// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smdialog.hh"
#include "smbutton.hh"

namespace SpectMorph
{

class LoadStereoDialog : public Dialog
{
public:
  enum class Result {
    LEFT,
    RIGHT,
    MIX
  };
  Signal<Result> signal_result;
  LoadStereoDialog (Window *window) :
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
    mk_label ("Convert file to mono?")->set_bold (true);
    yoffset += 4;
    mk_label ("The file you selected is a stereo file.");
    yoffset += 2;
    mk_label ("SpectMorph currently only supports mono samples.");


    double w = tw / 8 + 23;
    yoffset = 2;

    auto left_button = new Button (this, "Use left channel");
    grid.add_widget (left_button, w - 17, yoffset, 15, 3);
    connect (left_button->signal_clicked, [&](){ signal_result (Result::LEFT); on_accept();});
    yoffset += 3;

    auto right_button = new Button (this, "Use right channel");
    grid.add_widget (right_button, w - 17, yoffset, 15, 3);
    connect (right_button->signal_clicked, [&](){ signal_result (Result::RIGHT); on_accept();});
    yoffset += 3;

    auto mix_button = new Button (this, "Mix both channels");
    grid.add_widget (mix_button, w - 17, yoffset, 15, 3);
    connect (mix_button->signal_clicked, [&](){ signal_result (Result::MIX); on_accept();});
    yoffset += 3;

    auto cancel_button = new Button (this, "Cancel");
    grid.add_widget (cancel_button, w - 17, yoffset, 15, 3);
    connect (cancel_button->signal_clicked, [&](){ on_reject();});
    yoffset += 3;

    grid.add_widget (new VLine (this, Color (0.6, 0.6, 0.6), 2), tw / 8 + 4, 1, 1, yoffset);

    window->set_keyboard_focus (this);

    grid.add_widget (this, 0, 0, w, yoffset + 2);
  }
};

}
