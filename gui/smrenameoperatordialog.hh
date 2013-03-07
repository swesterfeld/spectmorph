// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_RENAME_OPERATOR_DIALOG_HH
#define SPECTMORPH_RENAME_OPERATOR_DIALOG_HH

#include "smmorphoperator.hh"

#include <QDialog>
#include <QLineEdit>

namespace SpectMorph
{

class RenameOperatorDialog : public QDialog
{
  Q_OBJECT

  MorphOperator *op;

  QLineEdit *new_name_edit;
  QPushButton *ok_button;
public:
  RenameOperatorDialog (QWidget *parent, MorphOperator *op);

  std::string new_name();

public slots:
  void on_name_changed();
};

}

#endif
