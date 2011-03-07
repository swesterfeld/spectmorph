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

using namespace SpectMorph;

using std::string;
using std::vector;

MorphOutputView::MorphOutputView (MorphOutput *morph_output, MainWindow *main_window) :
  MorphOperatorView (morph_output, main_window),
  block_channel_changed (false),
  morph_output (morph_output)
{
  for (int ch = 0; ch < 4; ch++)
    {
      ChannelView *chv = new ChannelView;
      chv->label.set_text (Birnet::string_printf ("Channel #%d", ch + 1));
      channel_table.attach (chv->label, 0, 1, ch, ch + 1, Gtk::SHRINK);
      channel_table.attach (chv->combobox, 1, 2, ch, ch + 1);
      channels.push_back (chv);

      chv->combobox.signal_changed().connect (sigc::mem_fun (*this, &MorphOutputView::on_channel_changed));
    }
  channel_table.set_spacings (10);
  channel_table.set_border_width (5);
  frame.add (channel_table);

  on_operators_changed();

  show_all_children();
}

MorphOutputView::~MorphOutputView()
{
  for (vector<ChannelView *>::iterator i = channels.begin(); i != channels.end(); i++)
    {
      delete *i;
    }
}

void
MorphOutputView::on_operators_changed()
{
  const vector<MorphOperator *>& ops = morph_output->morph_plan()->operators();

  block_channel_changed = true;
  for (size_t i = 0; i < channels.size(); i++)
    {
      Gtk::ComboBoxText& combo = channels[i]->combobox;
      combo.clear();

      MorphOperator *active_morph_op = morph_output->channel_op (i);
      for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
        {
          MorphOperator *morph_op = *oi;
          if (morph_op->output_type() == MorphOperator::OUTPUT_AUDIO)
            {
              combo.append_text (morph_op->name());
              if (morph_op == active_morph_op)
                combo.set_active_text (morph_op->name());
            }
        }
    }
  block_channel_changed = false;
}

void
MorphOutputView::on_channel_changed()
{
  if (block_channel_changed)
    return;

  for (size_t i = 0; i < channels.size(); i++)
    {
      const vector<MorphOperator *>& ops = morph_output->morph_plan()->operators();

      MorphOperator *active_morph_op = NULL;

      for (vector<MorphOperator *>::const_iterator oi = ops.begin(); oi != ops.end(); oi++)
        {
          MorphOperator *morph_op = *oi;
          if (morph_op->name() == channels[i]->combobox.get_active_text())
            active_morph_op = morph_op;
        }
      morph_output->set_channel_op (i, active_morph_op);
    }
}
