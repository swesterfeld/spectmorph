// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_VST_UI_HH
#define SPECTMORPH_VST_UI_HH

#include "smvstcommon.hh"

#include "smmorphplanwindow.hh"
#include "smmorphplancontrol.hh"

namespace SpectMorph
{

class VstPlugin;

class VstUI : public QObject
{
  Q_OBJECT

  ERect                 rectangle;
  MorphPlanWindow      *widget;
  MorphPlanControl     *control_widget;
  MorphPlanPtr          morph_plan;
  VstPlugin            *plugin;
  
public:
  VstUI (const std::string& filename, VstPlugin *plugin);

  bool open (WId win_id);
  bool getRect (ERect** rect);
  void close();
  void idle();

  int   save_state (char **ptr);
  void  load_state (char *ptr);

public slots:
  void on_plan_changed();
  void on_update_window_size();
  void on_change_volume (double new_volume);
};

}

#endif
