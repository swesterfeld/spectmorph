// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_VST_UI_HH
#define SPECTMORPH_VST_UI_HH

#include "smvstcommon.hh"

#include "smmorphplanwindow.hh"

namespace SpectMorph
{

class VstUI : public QObject
{
  Q_OBJECT

  ERect             rectangle;
  MorphPlanWindow  *widget;
  MorphPlanPtr      morph_plan;
public:
  VstUI (const std::string& filename);

  bool open (WId win_id);
  bool getRect (ERect** rect);
  void close();
  void idle();
};

}

#endif
