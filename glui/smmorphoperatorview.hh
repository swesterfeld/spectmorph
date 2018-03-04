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
#include "smtoolbutton.hh"
#include <functional>

namespace SpectMorph
{

class MorphPlanWindow;

class MorphOperatorView : public Frame
{
protected:
  ToolButton    *fold_button;
  ToolButton    *close_button;

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

    fold_button = new ToolButton (this);
    grid.add_widget (fold_button, 2, 1, 2, 2);
    connect (fold_button->signal_clicked, this, &MorphOperatorView::on_fold_clicked);

    close_button = new ToolButton (this, 'x');
    grid.add_widget (close_button, 39, 1, 2, 2);
    connect (close_button->signal_clicked, [=]() { m_op->morph_plan()->remove (m_op); });

    body_widget = new Widget (this);

    update_body_visible();
  }
  void
  hide_tool_buttons()
  {
    fold_button->set_visible (false);
    close_button->set_visible (false);
  }
  void
  on_fold_clicked()
  {
    m_op->set_folded (!m_op->folded());

    update_body_visible();

    signal_fold_changed();
  }
  void
  update_body_visible()
  {
    fold_button->symbol = m_op->folded() ? '>' : 'v';
    body_widget->set_visible (!m_op->folded());
  }
  virtual double
  view_height()
  {
    return 4;
  }
  virtual bool
  is_output()
  {
    return false;
  }
};

}

#endif
