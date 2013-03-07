// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_OSC_GUI_HH
#define SPECTMORPH_OSC_GUI_HH

#include "smmorphplanwindow.hh"

namespace SpectMorph
{

class OscGui : public QObject
{
  Q_OBJECT

  MorphPlanPtr      morph_plan;
  MorphPlanWindow  *window;
public:
  OscGui (MorphPlanPtr plan, const std::string& title);

public slots:
  void on_plan_changed();
};

}

#endif
