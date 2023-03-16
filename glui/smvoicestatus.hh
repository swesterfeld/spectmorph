// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smmodulationlist.hh"
#include "smmidisynth.hh"

namespace SpectMorph
{

class VoiceStatus
{
  std::map<uintptr_t, std::vector<VoiceOpValuesEvent::Voice>> control_value_map;
  bool                   control_value_map_clear = false;
  std::vector<uintptr_t> voices;
  std::vector<float>     controls[MorphPlan::N_CONTROL_INPUTS];

  float
  get_control_value (Property& property, int i, MorphOperator::ControlType control_type, MorphOperator *control_op)
  {
    if (control_type == MorphOperator::CONTROL_GUI)
      {
        return 2 * (property.get() - property.min()) / double (property.max() - property.min()) - 1;
      }
    if (control_type == MorphOperator::CONTROL_SIGNAL_1)
      {
        return controls[0][i];
      }
    if (control_type == MorphOperator::CONTROL_SIGNAL_2)
      {
        return controls[1][i];
      }
    if (control_type == MorphOperator::CONTROL_SIGNAL_3)
      {
        return controls[2][i];
      }
    if (control_type == MorphOperator::CONTROL_SIGNAL_4)
      {
        return controls[3][i];
      }
    if (control_type == MorphOperator::CONTROL_OP)
      {
        for (const auto& op_entry : control_value_map[voices[i]])
          if (op_entry.op == (uintptr_t) control_op)
            return op_entry.value;
      }
    return 0;
  }
public:
  bool
  process_notify_event (SynthNotifyEvent *ne)
  {
    bool changed = false;

    auto vo_values = dynamic_cast<VoiceOpValuesEvent *> (ne);
    if (vo_values)
      {
        if (control_value_map_clear)
          {
            control_value_map.clear();
            control_value_map_clear = false;
          }
        if (vo_values->voices.size())
          control_value_map[vo_values->voices[0].voice] = vo_values->voices;
      }
    auto av_status = dynamic_cast<ActiveVoiceStatusEvent *> (ne);
    if (av_status)
      {
        voices = av_status->voice;
        for (size_t i = 0; i < MorphPlan::N_CONTROL_INPUTS; i++)
          controls[i] = av_status->control[i];
        control_value_map_clear = true;
        changed = true;
      }
    return changed;
  }
  std::vector<float>
  get_values (Property& property)
  {
    std::vector<float> property_values;

    ModulationList *mod_list = property.modulation_list();

    for (size_t i = 0; i < voices.size(); i++)
      {
        float value = get_control_value (property, i, mod_list->main_control_type(), mod_list->main_control_op());

        for (size_t index = 0; index < mod_list->count(); index++)
          {
            const auto& mod_entry = (*mod_list)[index];
            const float mod_value_scale = 2; /* range [-1:1] */

            float mod_value = get_control_value (property, i, mod_entry.control_type, mod_entry.control_op.get());
            if (!mod_entry.bipolar)
              mod_value = 0.5 * (mod_value + 1);

            value += mod_value * mod_entry.amount * mod_value_scale;
          }

        property_values.push_back (std::clamp (value, -1.f, 1.f));
      }

    return property_values;
  }
  uint
  n_voices()
  {
    return voices.size();
  }
};

}
