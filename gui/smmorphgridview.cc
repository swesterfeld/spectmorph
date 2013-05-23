// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphgridview.hh"
#include "smmorphgridwidget.hh"
#include "smmorphplan.hh"
#include "smcomboboxoperator.hh"
#include "smutils.hh"

#include <QSpinBox>
#include <QLabel>
#include <QPainter>
#include <QStackedWidget>
#include <QPushButton>

using namespace SpectMorph;

using std::string;
using std::vector;
using std::set;

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

  zoom_spinbox = new QSpinBox();
  zoom_spinbox->setRange (1, 25);
  zoom_spinbox->setValue (morph_grid->zoom());

  grid->addWidget (new QLabel ("Zoom"), 1, 0);
  grid->addWidget (zoom_spinbox, 1, 1, 1, 2);

  op_combobox = new ComboBoxOperator (morph_grid->morph_plan(), &input_op_filter);

  x_ui = new MorphGridControlUI (this, morph_grid, MorphGridControlUI::CONTROL_X);
  grid->addWidget (new QLabel ("X Control"), 2, 0);
  grid->addWidget (x_ui->combobox, 2, 1, 1, 2);

  y_ui = new MorphGridControlUI (this, morph_grid, MorphGridControlUI::CONTROL_Y);
  grid->addWidget (new QLabel ("Y Control"), 3, 0);
  grid->addWidget (y_ui->combobox, 3, 1, 1, 2);

  grid->addWidget (new QLabel ("X Value"), 4, 0);
  grid->addWidget (x_ui->stack, 4, 1);
  grid->addWidget (x_ui->label, 4, 2);

  grid->addWidget (new QLabel ("Y Value"), 5, 0);
  grid->addWidget (y_ui->stack, 5, 1);
  grid->addWidget (y_ui->label, 5, 2);

  grid_widget = new MorphGridWidget (morph_grid);
  grid->addWidget (grid_widget, 6, 0, 1, 3);

  grid->addWidget (new QLabel ("Source"), 7, 0);
  grid->addWidget (op_combobox, 7, 1, 1, 2);

  delta_db_slider = new QSlider (Qt::Horizontal);
  delta_db_slider->setRange (-48000, 48000);
  delta_db_label = new QLabel (string_locale_printf ("%.1f dB", 0.0).c_str());

  grid->addWidget (new QLabel ("Volume"), 8, 0);
  grid->addWidget (delta_db_slider, 8, 1);
  grid->addWidget (delta_db_label, 8, 2);

  setLayout (grid);

  connect (grid_widget, SIGNAL (selection_changed()), this, SLOT (on_selection_changed()));
  connect (width_spinbox, SIGNAL (valueChanged (int)), this, SLOT (on_size_changed()));
  connect (height_spinbox, SIGNAL (valueChanged (int)), this, SLOT (on_size_changed()));
  connect (zoom_spinbox, SIGNAL (valueChanged (int)), this, SLOT (on_zoom_changed()));
  connect (op_combobox, SIGNAL (active_changed()), this, SLOT (on_operator_changed()));
  connect (delta_db_slider, SIGNAL (valueChanged(int)), this, SLOT (on_delta_db_changed (int)));
  connect (morph_grid->morph_plan(), SIGNAL (plan_changed()), this, SLOT (on_plan_changed()));
  connect (morph_grid->morph_plan(), SIGNAL (index_changed()), this, SLOT (on_index_changed()));

  on_index_changed();     // add instruments to op_combobox
  on_plan_changed();      // initial morphing slider/label setup
  on_selection_changed(); // initial selection
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
  delta_db_slider->setEnabled (morph_grid->has_selection());
  delta_db_label->setEnabled (morph_grid->has_selection());

  if (morph_grid->has_selection())
    {
      MorphGridNode node = morph_grid->input_node (morph_grid->selected_x(), morph_grid->selected_y());

      if (node.smset != "")
        {
          g_assert (node.op == NULL);
          op_combobox->set_active_str_choice (node.smset);
        }
      else
        {
          op_combobox->set_active (node.op);
        }
      delta_db_slider->setValue (lrint (node.delta_db * 1000));
    }
}

void
MorphGridView::on_operator_changed()
{
  if (morph_grid->has_selection())
    {
      MorphGridNode node = morph_grid->input_node (morph_grid->selected_x(), morph_grid->selected_y());

      node.op = op_combobox->active();
      node.smset = op_combobox->active_str_choice();

      morph_grid->set_input_node (morph_grid->selected_x(), morph_grid->selected_y(), node);
    }
}

void MorphGridView::on_delta_db_changed (int new_value)
{
  const double db = new_value / 1000.0;

  delta_db_label->setText (string_locale_printf ("%.1f dB", db).c_str());

  if (morph_grid->has_selection())
    {
      MorphGridNode node = morph_grid->input_node (morph_grid->selected_x(), morph_grid->selected_y());

      node.delta_db = db;

      morph_grid->set_input_node (morph_grid->selected_x(), morph_grid->selected_y(), node);
    }
}

void
MorphGridView::on_plan_changed()
{
  x_ui->slider->setValue (lrint (morph_grid->x_morphing() * 1000));
  y_ui->slider->setValue (lrint (morph_grid->y_morphing() * 1000));
  x_ui->label->setText (string_locale_printf ("%.2f", morph_grid->x_morphing()).c_str());
  y_ui->label->setText (string_locale_printf ("%.2f", morph_grid->y_morphing()).c_str());
}

void
MorphGridView::on_index_changed()
{
  op_combobox->clear_str_choices();

  vector<string> smsets = morph_grid->morph_plan()->index()->smsets();
  for (vector<string>::iterator si = smsets.begin(); si != smsets.end(); si++)
    op_combobox->add_str_choice (*si);

  set<string> smset_set (smsets.begin(), smsets.end());
  for (int x = 0; x < morph_grid->width(); x++)
    {
      for (int y = 0; y < morph_grid->height(); y++)
        {
          MorphGridNode node = morph_grid->input_node (x, y);

          if (node.smset != "" && !smset_set.count (node.smset))
            {
              // instrument not present in new index, remove
              // (probably should not be done in gui code)
              node.smset = "";
              morph_grid->set_input_node (x, y, node);
            }
        }
    }
}

void
MorphGridView::on_zoom_changed()
{
  morph_grid->set_zoom (zoom_spinbox->value());
}
