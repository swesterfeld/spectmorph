// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smaboutdialog.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"
#include "smbutton.hh"
#include "config.h"

using namespace SpectMorph;

AboutDialog::AboutDialog (Window *window) :
  Dialog (window)
{
  FixedGrid grid;

  auto title_label = new Label (this, "SpectMorph " PACKAGE_VERSION);
  title_label->set_bold (true);
  title_label->set_align (TextAlign::CENTER);

  grid.add_widget (title_label, 0, 0, 40, 4);
  grid.add_widget (this, 0, 0, 40, 18);

  double yoffset = 4;

  auto web_label = new Label (this, "Website: www.spectmorph.org");

  grid.add_widget (web_label, 10, yoffset, 40, 3);
  yoffset += 3;

  auto license_label = new Label (this, "License: GNU LGPL version 3");

  grid.add_widget (license_label, 10, yoffset, 40, 3);
  yoffset += 3;

  auto author_label = new Label (this, "Author: Stefan Westerfeld");

  grid.add_widget (author_label, 10, yoffset, 40, 3);
  yoffset += 4;

  auto ok_button = new Button (this, "Ok");
  grid.add_widget (ok_button, 15, yoffset, 10, 3);
  connect (ok_button->signal_clicked, this, &Dialog::on_accept);

  window->set_keyboard_focus (this);
}
