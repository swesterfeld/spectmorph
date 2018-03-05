// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_RENAME_OP_DIALOG_HH
#define SPECTMORPH_RENAME_OP_DIALOG_HH

#include "smwindow.hh"
#include "smdialog.hh"
#include "smmorphoperator.hh"
#include "smlineedit.hh"
#include "smbutton.hh"

namespace SpectMorph
{

class RenameOpDialog : public Dialog
{
protected:
  LineEdit *line_edit;

  Button   *ok_button;
  Button   *cancel_button;

public:
  RenameOpDialog (Window *parent, MorphOperator *op);

  void run (std::function<void (bool)> done_callback);
  std::string new_name();
};

}

#endif

