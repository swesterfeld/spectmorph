// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_OPERATOR_VIEW_HH
#define SPECTMORPH_MORPH_OPERATOR_VIEW_HH

#include "smwindow.hh"
#include "smlabel.hh"
#include "smfixedgrid.hh"
#include "smframe.hh"
#include "smslider.hh"
#include <functional>

namespace SpectMorph
{

struct MorphOperatorView : public Frame
{
public:
  MorphOperatorView (Widget *parent, MorphOperator *op) :
    Frame (parent)
  {
    FixedGrid grid;

    // FIXME: need update (signal) on_operators_changed
    std::string title = op->type_name() + ": " + op->name();

    Label *label = new Label (this, title);
    label->align = TextAlign::CENTER;
    label->bold  = true;
    grid.add_widget (label, 0, 0, 43, 4);

    Label *slider_label = new Label (this, "Attack");
    Slider *slider = new Slider (this, 0.5);
    Label *value_label = new Label (this, "50%");

    int yoffset = 4;
    grid.add_widget (slider_label, 3, yoffset, 7, 2);
    grid.add_widget (slider,  10, yoffset, 27, 2);
    grid.add_widget (value_label, 38, yoffset, 5, 2);
  }
};

}

#endif
