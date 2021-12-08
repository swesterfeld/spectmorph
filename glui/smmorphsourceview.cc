// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmorphsourceview.hh"
#include "smmorphplan.hh"

#include "smlabel.hh"
#include "smoperatorlayout.hh"

using namespace SpectMorph;

using std::string;
using std::vector;

MorphSourceView::MorphSourceView (Widget *parent, MorphSource *morph_source, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (parent, morph_source, morph_plan_window),
  morph_source (morph_source)
{
  OperatorLayout op_layout;

  Label *instrument_label = new Label (body_widget, "Instrument");
  instrument_combobox = new ComboBox (body_widget);

  op_layout.add_row (3, instrument_label, instrument_combobox);
  op_layout.activate();

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

  auto groups = morph_source->morph_plan()->index()->groups();
  for (auto group : groups)
    {
      instrument_combobox->add_item (ComboBoxItem (group.group, true));
      for (auto instrument : group.instruments)
        {
          instrument_combobox->add_item (ComboBoxItem (instrument.label));
        }
    }

  string label = morph_source->morph_plan()->index()->smset_to_label (morph_source->smset());
  if (!label.empty())
    {
      instrument_combobox->set_text (label);
    }
  else
    {
      // use first instrument as default (could be moved from gui code to model)
      auto smsets = morph_source->morph_plan()->index()->smsets();
      if (!smsets.empty())
        {
          string label = morph_source->morph_plan()->index()->smset_to_label (smsets[0]);
          instrument_combobox->set_text (label);
        }
      on_instrument_changed();
    }
}

void
MorphSourceView::on_instrument_changed()
{
  string smset = morph_source->morph_plan()->index()->label_to_smset (instrument_combobox->text());

  // smset could be empty here (label not found) - but that is ok, then we unset source instrument
  morph_source->set_smset (smset);
}
