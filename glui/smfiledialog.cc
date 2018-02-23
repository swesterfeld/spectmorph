// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smfiledialog.hh"
#include "smbutton.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"

using namespace SpectMorph;

FileDialog::FileDialog (Window *parent_window)
 : Window (640, 480, false, true)
{
  FixedGrid grid;

  show();
  set_close_callback ([=]() {
    parent_window->on_file_selected ("");
  });

  Label  *fn_label = new Label (this, "File Name");
  Button *open_button = new Button (this, "Open File");
  Button *cancel_button = new Button (this, "Cancel");

  double w8 = width / 8;
  double h8 = height / 8;
  grid.add_widget (fn_label, 2, h8 - 5, 10, 4);
  grid.add_widget (open_button, w8 - 11, h8 - 5, 10, 4);
  grid.add_widget (cancel_button, w8 - 22, h8 - 5, 10, 4);
}
