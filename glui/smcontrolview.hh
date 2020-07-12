// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_CONTROL_VIEW_HH
#define SPECTMORPH_CONTROL_VIEW_HH

namespace SpectMorph
{

class ControlView : public SignalReceiver
{
  struct Entry
  {
    MorphOperator::ControlType ctype;
    std::string                text;
  };
  const std::vector<Entry> entries =
    {
      { MorphOperator::CONTROL_GUI, "Gui Slider"},
      { MorphOperator::CONTROL_SIGNAL_1, "Control Signal #1"},
      { MorphOperator::CONTROL_SIGNAL_2, "Control Signal #2"},
      { MorphOperator::CONTROL_SIGNAL_3, "Control Signal #3"},
      { MorphOperator::CONTROL_SIGNAL_4, "Control Signal #4"}
    };
  ComboBoxOperator *control_combobox = nullptr;
public:
  ComboBoxOperator *
  create_combobox (Widget *parent, MorphOperator *op, MorphOperator::ControlType initial_type, MorphOperator *initial_op)
  {
    auto control_operator_filter = ComboBoxOperator::make_filter (op, MorphOperator::OUTPUT_CONTROL);
    control_combobox = new ComboBoxOperator (parent, op->morph_plan(), control_operator_filter);

    for (auto entry : entries)
      {
        control_combobox->add_str_choice (entry.text);
        if (entry.ctype == initial_type)
          control_combobox->set_active_str_choice (entry.text);
      }
    if (initial_type == MorphOperator::CONTROL_OP)
      control_combobox->set_active (initial_op);
    control_combobox->set_none_ok (false);

    connect (control_combobox->signal_item_changed, [this]() { signal_control_changed(); });
    return control_combobox;
  }
  MorphOperator *
  op() const
  {
    return control_combobox->active();
  }
  MorphOperator::ControlType
  control_type()
  {
    if (control_combobox->active())
      return MorphOperator::CONTROL_OP;

    std::string active_text = control_combobox->active_str_choice();
    for (auto entry : entries)
      {
        if (entry.text == active_text)
          return entry.ctype;
      }
    /* in principle this cannot be reached but we want to fail gracefully */
    return MorphOperator::CONTROL_GUI;
  }
  Signal<> signal_control_changed;
};

}

#endif
