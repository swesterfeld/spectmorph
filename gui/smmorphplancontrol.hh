// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MORPH_PLAN_CONTROL_HH
#define SPECTMORPH_MORPH_PLAN_CONTROL_HH

#include "smmorphplan.hh"
#include "smled.hh"

#include <QGroupBox>
#include <QLabel>
#include <QSlider>

namespace SpectMorph
{

class MorphPlanControl : public QGroupBox
{
  Q_OBJECT

  MorphPlanPtr  morph_plan;
  QLabel       *volume_value_label;
  QSlider      *volume_slider;
  Led          *midi_led;
  QLabel       *inst_status;

public:
  enum Features {
    ALL_WIDGETS,
    NO_VOLUME
  };
  MorphPlanControl (MorphPlanPtr plan, Features f = ALL_WIDGETS);

  void set_volume (double volume);
  void set_led (bool on);

signals:
  void change_volume (double volume);

public slots:
  void on_index_changed();
  void on_volume_changed (int new_volume);
};

}

#endif
