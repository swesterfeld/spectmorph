// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LV2_UI_HH
#define SPECTMORPH_LV2_UI_HH

#include "smmorphplanwindow.hh"
#include "smlv2common.hh"

namespace SpectMorph
{

class SpectMorphLV2UI : public QObject,
                        public SpectMorph::LV2Common
{
  Q_OBJECT

public:
  SpectMorphLV2UI();

  MorphPlanWindow      *window;
  MorphPlanPtr          morph_plan;

  LV2_Atom_Forge        forge;
  LV2UI_Write_Function  write;
  LV2UI_Controller      controller;

public slots:
  void on_plan_changed();
};

}

#endif /* SPECTMORPH_LV2_UI_HH */

