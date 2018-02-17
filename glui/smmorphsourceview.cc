// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphsourceview.hh"
#include "smmorphplan.hh"

#include "smlabel.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphSourceView::MorphSourceView (Widget *parent, MorphSource *morph_source, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_source, morph_plan_window),
  morph_source (morph_source)
{
  FixedGrid grid;

  Label *instrument_label = new Label (body_widget, "Instrument");
  instrument_combobox = new ComboBox (body_widget);

  int yoffset = 0;
  grid.add_widget (instrument_label, 0, yoffset, 9, 3);
  grid.add_widget (instrument_combobox, 9, yoffset, 30, 3);

  on_index_changed();

  connect (morph_source->morph_plan()->signal_index_changed, this, &MorphSourceView::on_index_changed);
  connect (instrument_combobox->signal_item_changed, this, &MorphSourceView::on_instrument_changed);
}

double
MorphSourceView::view_height()
{
  return 8;
}

void
MorphSourceView::on_index_changed()
{
  instrument_combobox->clear();

  vector<string> smsets = morph_source->morph_plan()->index()->smsets();
  for (auto s : smsets)
    instrument_combobox->add_item (ComboBoxItem (s));

  auto it = find (smsets.begin(), smsets.end(), morph_source->smset());
  if (it != smsets.end())
    {
      instrument_combobox->set_text (morph_source->smset());
    }
  else
    {
      // use first instrument as default (could be moved from gui code to model)
      if (!smsets.empty())
        instrument_combobox->set_text (smsets[0]);
      on_instrument_changed();
    }
}

void
MorphSourceView::on_instrument_changed()
{
  morph_source->set_smset (instrument_combobox->text());
}
