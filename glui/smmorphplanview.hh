// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_VIEW_HH
#define SPECTMORPH_MORPH_PLAN_VIEW_HH

#include "smwindow.hh"
#include "smlabel.hh"
#include "smfixedgrid.hh"
#include "smmorphoperatorview.hh"
#include <functional>

namespace SpectMorph
{

struct MorphPlanView : public Widget
{
public:
  MorphPlanView (Widget *parent, MorphPlan *morph_plan) :
    Widget (parent)
  {
    FixedGrid grid;

    for (auto op : morph_plan->operators())
      {
        MorphOperatorView *op_view = new MorphOperatorView (this, op);
        grid.add_widget (op_view, 0, y, 43, 7);
        y += 8;
#if 0
        MorphOperatorView *op_view = MorphOperatorView::create (*oi, morph_plan_window);
        vbox->addWidget (op_view);
        m_op_views.push_back (op_view);

        connect (op_view, SIGNAL (move_indication (MorphOperator *)), this, SLOT (on_move_indication (MorphOperator *)));
        connect (op_view, SIGNAL (need_resize()), morph_plan_window, SLOT (on_need_resize()));

        MoveIndicator *indicator = new MoveIndicator();
        vbox->addWidget (indicator);

        move_indicators.push_back (indicator);
#endif
      }
  }
};

}

#endif
