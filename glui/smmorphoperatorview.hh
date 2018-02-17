// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_VIEW_HH
#define SPECTMORPH_MORPH_OPERATOR_VIEW_HH

#include "smwindow.hh"
#include "smlabel.hh"
#include "smfixedgrid.hh"
#include "smframe.hh"
#include "smslider.hh"
#include "smmorphplan.hh"
#include "smmorphplanwindow.hh"
#include "smfoldbutton.hh"
#include <functional>

namespace SpectMorph
{

class MorphPlanWindow;

class MorphOperatorView : public Frame
{
protected:
  FoldButton    *fold_button;

public:
  Widget        *body_widget;
  MorphOperator *m_op;

  Signal<> signal_fold_changed;

  MorphOperatorView (Widget *parent, MorphOperator *op, MorphPlanWindow *window) :
    Frame (parent),
    m_op (op)
  {
    FixedGrid grid;

    // FIXME: need update (signal) on_operators_changed
    std::string title = op->type_name() + ": " + op->name();

    Label *label = new Label (this, title);
    label->align = TextAlign::CENTER;
    label->bold  = true;
    grid.add_widget (label, 0, 0, 43, 4);

    fold_button = new FoldButton (this, m_op->folded());
    grid.add_widget (fold_button, 2, 1, 2, 2);
    connect (fold_button->signal_clicked, this, &MorphOperatorView::on_fold_clicked);

    body_widget = new Widget (this);

    update_body_visible();
  }
  void
  on_fold_clicked()
  {
    m_op->set_folded (fold_button->folded);

    update_body_visible();

    signal_fold_changed();
  }
  void
  update_body_visible()
  {
    body_widget->set_visible (!m_op->folded());
  }
  virtual double
  view_height()
  {
    return 4;
  }
};

}

#endif
