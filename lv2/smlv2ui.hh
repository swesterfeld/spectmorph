// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LV2_UI_HH
#define SPECTMORPH_LV2_UI_HH

#include "smmorphplanwindow.hh"
#include "smmorphplancontrol.hh"
#include "smlv2common.hh"

namespace SpectMorph
{

class LV2UI : public QObject,
              public LV2Common
{
  Q_OBJECT

public:
  LV2UI();
  ~LV2UI();

  MorphPlanWindow      *window;
  MorphPlanControl     *control_widget;
  MorphPlanPtr          morph_plan;

  LV2_Atom_Forge        forge;
  LV2UI_Write_Function  write;
  LV2UI_Controller      controller;

public slots:
  void on_plan_changed();
  void on_volume_changed (double new_volume);
};

}

#endif /* SPECTMORPH_LV2_UI_HH */

