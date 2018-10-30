// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SYNTH_INTERFACE_HH
#define SPECTMORPH_SYNTH_INTERFACE_HH

#include "smmidisynth.hh"
namespace SpectMorph
{

class SynthInterface
{
public:
  virtual
  ~SynthInterface()
  {
  }
  void
  synth_inst_edit_update (bool active, const std::string& filename, bool original_samples)
  {
    synth_take_control_event (new InstEditUpdate (active, filename, original_samples));
  }
  void
  synth_inst_edit_note (int note, bool on)
  {
    synth_take_control_event (new InstEditNote (note, on));
  }
  virtual void synth_take_control_event (SynthControlEvent *event) = 0;
  virtual double synth_get_current_pos()
  {
    return -1;
  }
  Signal<SynthNotifyEvent *> signal_notify_event;
};

}

#endif
