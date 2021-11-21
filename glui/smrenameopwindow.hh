// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_RENAME_OP_WINDOW_HH
#define SPECTMORPH_RENAME_OP_WINDOW_HH

#include "smwindow.hh"
#include "smdialog.hh"
#include "smmorphoperator.hh"
#include "smlineedit.hh"
#include "smbutton.hh"

namespace SpectMorph
{

class RenameOpWindow : public Window
{
protected:
  Window         *parent_window;
  MorphOperator  *m_op;
  LineEdit       *line_edit;

  Button         *ok_button;
  Button         *cancel_button;

  void on_accept();
  void on_reject();

  RenameOpWindow (Window *parent, MorphOperator *op);
public:
  static void create (Window *parent, MorphOperator *op);
};

}

#endif

