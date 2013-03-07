// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smmorphsourceview.hh"
#include "smmorphplan.hh"

#include <QComboBox>
#include <QLabel>

using namespace SpectMorph;

using std::string;
using std::vector;

MorphSourceView::MorphSourceView (MorphSource *morph_source, MorphPlanWindow *morph_plan_window) :
  MorphOperatorView (morph_source, morph_plan_window),
  morph_source (morph_source)
{
  QLabel *instrument_label = new QLabel ("Instrument");
  instrument_combobox = new QComboBox();

  QHBoxLayout *hbox = new QHBoxLayout();
  hbox->addWidget (instrument_label);
  hbox->addWidget (instrument_combobox, 1);

  setLayout (hbox);

  on_index_changed();
  connect (morph_source->morph_plan(), SIGNAL (index_changed()), this, SLOT (on_index_changed()));
  connect (instrument_combobox, SIGNAL (currentIndexChanged (int)), this, SLOT (on_instrument_changed()));
}

void
MorphSourceView::on_index_changed()
{
  instrument_combobox->clear();

  vector<string> smsets = morph_source->morph_plan()->index()->smsets();
  for (vector<string>::iterator si = smsets.begin(); si != smsets.end(); si++)
    instrument_combobox->addItem ((*si).c_str());

  int index = instrument_combobox->findText (morph_source->smset().c_str());
  if (index >= 0)
    instrument_combobox->setCurrentIndex (index);
}

void
MorphSourceView::on_instrument_changed()
{
  morph_source->set_smset (instrument_combobox->currentText().toLatin1().data());
}
