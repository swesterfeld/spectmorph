// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_VIEW_HH
#define SPECTMORPH_MORPH_OPERATOR_VIEW_HH

#include "smwindow.hh"
#include "smlabel.hh"
#include "smfixedgrid.hh"
#include "smframe.hh"
#include <functional>

namespace SpectMorph
{

struct MorphOperatorView : public Frame
{
public:
  MorphOperatorView (Widget *parent, FixedGrid& grid, MorphOperator *op) :
    Frame (parent)
  {
    Label *label = new Label (this, op->type());
    grid.add_widget (label, 1, 1, 20, 2);
  }
};

}

#endif
