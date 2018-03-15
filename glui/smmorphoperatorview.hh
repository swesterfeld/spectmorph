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
#include "smrenameopdialog.hh"
#include <functional>
#include <sys/time.h>

namespace SpectMorph
{

class MorphOperatorTitle : public Label
{
  bool in_move = false;
  double last_press_time = 0;

  /* FIXME: double click detection: move to toolkit */
  static double
  gettime()
  {
    timeval tv;
    gettimeofday (&tv, 0);

    return tv.tv_sec + tv.tv_usec / 1000000.0;
  }

public:
  MorphOperatorTitle (Widget *parent, const std::string& text) :
    Label (parent, text)
  {
  }
  void
  mouse_press (double x, double y) override
  {
    const bool double_click = gettime() - last_press_time < 0.3;
    last_press_time = gettime();

    if (double_click)
      {
        signal_rename();
      }
    else
      {
        in_move = true;

        signal_move (abs_y() + y);
      }
  }
  void
  motion (double x, double y) override
  {
    if (in_move)
      signal_move (abs_y() + y);
  }
  void
  mouse_release (double x, double y) override
  {
    if (in_move)
      {
        signal_end_move (abs_y() + y);
        in_move = false;
      }
  }
  Signal<double> signal_move;
  Signal<double> signal_end_move;
  Signal<>       signal_rename;
};
class MorphPlanWindow;

class MorphOperatorView : public Frame
{
protected:
  ToolButton         *fold_button;
  ToolButton         *close_button;
  MorphOperatorTitle *title_label;

  MorphPlanWindow *morph_plan_window;
  MorphOperator *m_op;
  MorphOperator *move_start_next;

public:
  Widget        *body_widget;

  Signal<>                      signal_size_changed;
  Signal<MorphOperator *, bool> signal_move_indication;

  MorphOperatorView (Widget *parent, MorphOperator *op, MorphPlanWindow *window);

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

    signal_size_changed();
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
  MorphOperator *
  op()
  {
    return m_op;
  }

  void on_move (double y);
  void on_end_move (double y);
  void on_rename();
  void on_operators_changed();
};

}

#endif
