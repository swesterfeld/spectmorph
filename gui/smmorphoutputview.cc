/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smmorphoutputview.hh"
#include "smmorphplan.hh"
#include <birnet/birnet.hh>

#include <QLabel>

using namespace SpectMorph;

using std::string;
using std::vector;

namespace {

struct MyOperatorFilter : public OperatorFilter
{
  bool filter (MorphOperator *op)
  {
    return (op->output_type() == MorphOperator::OUTPUT_AUDIO);
  }
} op_filter_instance;

}

MorphOutputView::MorphOutputView (MorphOutput *morph_output, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_output, morph_plan_window),
  morph_output (morph_output)
{
  QGridLayout *grid_layout = new QGridLayout();
  grid_layout->setColumnStretch (1, 1);

  for (int ch = 0; ch < 4; ch++)
    {
      ChannelView *chv = new ChannelView();
      chv->label = new QLabel (Birnet::string_printf ("Channel #%d", ch + 1).c_str());
      chv->combobox = new ComboBoxOperator (morph_output->morph_plan(), &op_filter_instance);

      grid_layout->addWidget (chv->label, ch, 0);
      grid_layout->addWidget (chv->combobox, ch, 1);
      channels.push_back (chv);

      chv->combobox->set_active (morph_output->channel_op (ch));

      connect (chv->combobox, SIGNAL (active_changed()), this, SLOT (on_operator_changed()));
    }
  QCheckBox *sines_check_box = new QCheckBox ("Enable Sine Synthesis");
  sines_check_box->setChecked (morph_output->sines());
  grid_layout->addWidget (sines_check_box, 4, 0, 1, 2);

  QCheckBox *noise_check_box = new QCheckBox ("Enable Noise Synthesis");
  noise_check_box->setChecked (morph_output->noise());
  grid_layout->addWidget (noise_check_box, 5, 0, 1, 2);

  connect (sines_check_box, SIGNAL (toggled (bool)), this, SLOT (on_sines_changed (bool)));
  connect (noise_check_box, SIGNAL (toggled (bool)), this, SLOT (on_noise_changed (bool)));
  setLayout (grid_layout);
}

void
MorphOutputView::on_sines_changed (bool new_value)
{
  morph_output->set_sines (new_value);
}

void
MorphOutputView::on_noise_changed (bool new_value)
{
  morph_output->set_noise (new_value);
}

void
MorphOutputView::on_operator_changed()
{
  for (size_t i = 0; i < channels.size(); i++)
    {
      morph_output->set_channel_op (i, channels[i]->combobox->active());
    }
}
