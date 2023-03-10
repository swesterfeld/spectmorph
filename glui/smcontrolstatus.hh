// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

namespace SpectMorph
{

class ControlStatus : public Widget
{
  std::vector<float> voices;
  Property& property;
  ModulationList *mod_list = nullptr;
  std::map<uintptr_t, std::vector<VoiceOpValuesEvent::Voice>> control_value_map;

  static constexpr double RADIUS = 6;
  double
  value_pos (double v)
  {
    double X = 8;
    return RADIUS + X + (width() - (RADIUS + X) * 2) * (v + 1) / 2;
  }
public:
  ControlStatus (Widget *parent, Property& property) :
    Widget (parent),
    property (property)
  {
    mod_list = property.modulation_list();
  }
  void
  redraw_voices()
  {
    for (auto v : voices)
      update (value_pos (v) - RADIUS - 1, height() / 2 - RADIUS - 1, RADIUS * 2 + 2, RADIUS * 2 + 2);
  }
  void
  reset_voices()
  {
    redraw_voices();
    voices.clear();
  }
  void
  add_voice (float value)
  {
    voices.push_back (value);
    redraw_voices();
  }
  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);
    double space = 2;
    du.round_box (0, space, width(), height() - 2 * space, 1, 5, Color (0.4, 0.4, 0.4), Color (0.3, 0.3, 0.3));

    Color vcolor = Color (0.7, 0.7, 0.7);

    // circle
    for (auto v : voices)
      {
        cairo_arc (cr, value_pos (v), height() / 2 , RADIUS, 0, 2 * M_PI);
        du.set_color (vcolor);
        cairo_fill_preserve (cr);
      }
  }
  void
  on_synth_notify_event (SynthNotifyEvent *ne)
  {
    auto vo_values = dynamic_cast<VoiceOpValuesEvent *> (ne);
    if (vo_values)
      {
        if (vo_values->voices.size())
          control_value_map[vo_values->voices[0].voice] = vo_values->voices;
      }
    auto av_status = dynamic_cast<ActiveVoiceStatusEvent *> (ne);
    if (av_status)
      {
        reset_voices();
        for (size_t i = 0; i < av_status->voice.size(); i++)
          {
            float value = get_control_value (av_status, i, mod_list->main_control_type(), mod_list->main_control_op());

            for (size_t index = 0; index < mod_list->count(); index++)
              {
                const auto& mod_entry = (*mod_list)[index];
                const float mod_value_scale = 2; /* range [-1:1] */

                float mod_value = get_control_value (av_status, i, mod_entry.control_type, mod_entry.control_op.get());
                if (!mod_entry.bipolar)
                  mod_value = 0.5 * (mod_value + 1);

                value += mod_value * mod_entry.amount * mod_value_scale;
              }

            add_voice (std::clamp (value, -1.f, 1.f));
          }

        control_value_map.clear();
      }
  }
  float
  get_control_value (ActiveVoiceStatusEvent *av_status, int i, MorphOperator::ControlType control_type, MorphOperator *control_op)
  {
    if (control_type == MorphOperator::CONTROL_GUI)
      {
        return 2 * (this->property.get() - this->property.min()) / double (this->property.max() - this->property.min()) - 1;
      }
    if (control_type == MorphOperator::CONTROL_SIGNAL_1)
      {
        return av_status->control[0][i];
      }
    if (control_type == MorphOperator::CONTROL_SIGNAL_2)
      {
        return av_status->control[1][i];
      }
    if (control_type == MorphOperator::CONTROL_SIGNAL_3)
      {
        return av_status->control[2][i];
      }
    if (control_type == MorphOperator::CONTROL_SIGNAL_4)
      {
        return av_status->control[3][i];
      }
    if (control_type == MorphOperator::CONTROL_OP)
      {
        for (const auto& op_entry : control_value_map[av_status->voice[i]])
          if (op_entry.op == (uintptr_t) control_op)
            return op_entry.value;
      }
    return 0;
  }
};

}
