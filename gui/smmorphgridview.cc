// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridview.hh"
#include "smmorphgridwidget.hh"
#include "smmorphplan.hh"
#include "smcomboboxoperator.hh"

#include <QSpinBox>
#include <QLabel>
#include <QPainter>
#include <QStackedWidget>

using namespace SpectMorph;

using std::string;

#define CONTROL_TEXT_GUI "Gui Slider"
#define CONTROL_TEXT_1   "Control Signal #1"
#define CONTROL_TEXT_2   "Control Signal #2"

MorphGridControlUI::MorphGridControlUI (MorphGridView *parent, MorphGrid *morph_grid, ControlXYType ctl_xy) :
  control_op_filter (morph_grid, MorphOperator::OUTPUT_CONTROL),
  morph_grid (morph_grid),
  ctl_xy (ctl_xy)
{
  combobox = new ComboBoxOperator (morph_grid->morph_plan(), &control_op_filter);
  combobox->add_str_choice (CONTROL_TEXT_GUI);
  combobox->add_str_choice (CONTROL_TEXT_1);
  combobox->add_str_choice (CONTROL_TEXT_2);
  combobox->set_none_ok (false);

  /* restore initial combobox state */
  MorphGrid::ControlType control_type = (ctl_xy == CONTROL_X) ?
                                        morph_grid->x_control_type() :
                                        morph_grid->y_control_type();

  if (control_type == MorphGrid::CONTROL_GUI)
    combobox->set_active_str_choice (CONTROL_TEXT_GUI);
  else if (control_type == MorphGrid::CONTROL_SIGNAL_1)
    combobox->set_active_str_choice (CONTROL_TEXT_1);
  else if (control_type == MorphGrid::CONTROL_SIGNAL_2)
    combobox->set_active_str_choice (CONTROL_TEXT_2);
  else if (control_type == MorphGrid::CONTROL_OP)
    {
      if (ctl_xy == CONTROL_X)
        {
          combobox->set_active (morph_grid->x_control_op());
        }
      else
        {
          combobox->set_active (morph_grid->y_control_op());
        }
    }
  else
    {
      assert (false);
    }

  stack = new QStackedWidget();

  slider = new QSlider (Qt::Horizontal);
  slider->setRange (-1000, 1000);
  label = new QLabel();

  /* restore slider value from operator */
  if (ctl_xy == CONTROL_X)
    slider->setValue (morph_grid->x_morphing() * 1000);
  else
    slider->setValue (morph_grid->y_morphing() * 1000);

  stack->addWidget (new QLabel ("from control input"));
  stack->addWidget (slider);

  connect (slider, SIGNAL (valueChanged(int)), this, SLOT (on_slider_changed()));
  connect (combobox, SIGNAL (active_changed()), this, SLOT (on_combobox_changed()));

  // initial slider state
  on_slider_changed();
  on_combobox_changed();
}

void
MorphGridControlUI::on_slider_changed()
{
  value = slider->value() / 1000.0;

  if (ctl_xy == CONTROL_X)
    morph_grid->set_x_morphing (value);
  else
    morph_grid->set_y_morphing (value);
}

void
MorphGridControlUI::on_combobox_changed()
{
  int stack_index = 0;
  MorphOperator *op = combobox->active();
  if (op)
    {
      if (ctl_xy == CONTROL_X)
        {
          morph_grid->set_x_control_op (op);
          morph_grid->set_x_control_type (MorphGrid::CONTROL_OP);
        }
      else
        {
          morph_grid->set_y_control_op (op);
          morph_grid->set_y_control_type (MorphGrid::CONTROL_OP);
        }
    }
  else
    {
      string text = combobox->active_str_choice();
      MorphGrid::ControlType new_type;

      if (text == CONTROL_TEXT_GUI)
        {
          new_type = MorphGrid::CONTROL_GUI;

          stack_index = 1;
        }
      else if (text == CONTROL_TEXT_1)
        new_type = MorphGrid::CONTROL_SIGNAL_1;
      else if (text == CONTROL_TEXT_2)
        new_type = MorphGrid::CONTROL_SIGNAL_2;
      else
        {
          assert (false);
        }
      if (ctl_xy == CONTROL_X)
        morph_grid->set_x_control_type (new_type);
      else
        morph_grid->set_y_control_type (new_type);
    }
  stack->setCurrentIndex (stack_index);
  if (stack_index == 1)
    label->show();
  else
    label->hide();
}

MorphGridView::MorphGridView (MorphGrid *morph_grid, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_grid, morph_plan_window),
  morph_grid (morph_grid),
  input_op_filter (morph_grid, MorphOperator::OUTPUT_AUDIO)
{
  width_spinbox = new QSpinBox();
  height_spinbox = new QSpinBox();

  width_spinbox->setRange (1, 16);
  height_spinbox->setRange (1, 16);

  width_spinbox->setValue (morph_grid->width());
  height_spinbox->setValue (morph_grid->height());

  QHBoxLayout *hbox = new QHBoxLayout();
  hbox->addWidget (new QLabel ("Width"));
  hbox->addWidget (width_spinbox);
  hbox->addWidget (new QLabel ("Height"));
  hbox->addWidget (height_spinbox);

  QGridLayout *grid = new QGridLayout();
  grid->addLayout (hbox, 0, 0, 1, 3);

  op_combobox = new ComboBoxOperator (morph_grid->morph_plan(), &input_op_filter);

  x_ui = new MorphGridControlUI (this, morph_grid, MorphGridControlUI::CONTROL_X);
  grid->addWidget (new QLabel ("X Control"), 1, 0);
  grid->addWidget (x_ui->combobox, 1, 1, 1, 2);

  y_ui = new MorphGridControlUI (this, morph_grid, MorphGridControlUI::CONTROL_Y);
  grid->addWidget (new QLabel ("Y Control"), 2, 0);
  grid->addWidget (y_ui->combobox, 2, 1, 1, 2);

  grid->addWidget (new QLabel ("X Value"), 3, 0);
  grid->addWidget (x_ui->stack, 3, 1);
  grid->addWidget (x_ui->label, 3, 2);

  grid->addWidget (new QLabel ("Y Value"), 4, 0);
  grid->addWidget (y_ui->stack, 4, 1);
  grid->addWidget (y_ui->label, 4, 2);

  QHBoxLayout *bottom_hbox = new QHBoxLayout();
  bottom_hbox->addWidget (new QLabel ("Instrument/Source"));
  bottom_hbox->addWidget (op_combobox);

  grid_widget = new MorphGridWidget (morph_grid);
  QVBoxLayout *vbox = new QVBoxLayout();
  vbox->addLayout (grid);
  vbox->addWidget (grid_widget);
  vbox->addLayout (bottom_hbox);
  setLayout (vbox);

  connect (grid_widget, SIGNAL (selection_changed()), this, SLOT (on_selection_changed()));
  connect (width_spinbox, SIGNAL (valueChanged (int)), this, SLOT (on_size_changed()));
  connect (height_spinbox, SIGNAL (valueChanged (int)), this, SLOT (on_size_changed()));
  connect (op_combobox, SIGNAL (active_changed()), this, SLOT (on_operator_changed()));
  connect (morph_grid->morph_plan(), SIGNAL (plan_changed()), this, SLOT (on_plan_changed()));

  on_plan_changed(); // initial morphing slider/label setup
}

void
MorphGridView::on_size_changed()
{
  morph_grid->set_width (width_spinbox->value());
  morph_grid->set_height (height_spinbox->value());
}

void
MorphGridView::on_selection_changed()
{
  op_combobox->setEnabled (morph_grid->has_selection());

  if (morph_grid->has_selection())
    {
      MorphGridNode node = morph_grid->input_node (morph_grid->selected_x(), morph_grid->selected_y());
      op_combobox->set_active (node.op);
    }
}

void
MorphGridView::on_operator_changed()
{
  if (morph_grid->has_selection())
    {
      MorphGridNode node;

      node.op = op_combobox->active();
      morph_grid->set_input_node (morph_grid->selected_x(), morph_grid->selected_y(), node);
    }
}

void
MorphGridView::on_plan_changed()
{
  x_ui->slider->setValue (lrint (morph_grid->x_morphing() * 1000));
  y_ui->slider->setValue (lrint (morph_grid->y_morphing() * 1000));
  x_ui->label->setText (Birnet::string_printf ("%.2f", morph_grid->x_morphing()).c_str());
  y_ui->label->setText (Birnet::string_printf ("%.2f", morph_grid->y_morphing()).c_str());
}
