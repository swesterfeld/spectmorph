// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_PROPERTY_VIEW_EDIT_HH
#define SPECTMORPH_PROPERTY_VIEW_EDIT_HH

#include "smwindow.hh"
#include "smbutton.hh"
#include "smlineedit.hh"
#include "smcontrolview.hh"
#include "smparamlabel.hh"
#include "smtoolbutton.hh"
#include "smscrollview.hh"
#include "smmorphplanwindow.hh"
#include "smcontrolstatus.hh"

namespace SpectMorph
{

class PropertyViewEdit : public Window
{
protected:
  MorphPlanWindow *parent_window;
  Property&       property;
  ModulationList *mod_list = nullptr;

  Button         *add_mod_button;
  ControlStatus  *control_status = nullptr;
  Label          *value_label = nullptr;
  Slider         *slider = nullptr;
  LineEdit       *line_edit;
  bool            line_edit_changed = false;
  ControlView     main_control_view;
  Widget         *mod_list_hdr = nullptr;
  Label          *mod_list_label = nullptr;
  ScrollView     *scroll_view;
  Widget         *scroll_widget;

  std::vector<ControlView *> control_views;
  std::vector<Widget *> mod_widgets;

  int
  window_height (Property& property)
  {
    if (property.modulation_list())
      return 296;
    else
      return 72;
  }
  PropertyViewEdit (MorphPlanWindow *parent, Property& property) :
    Window (*parent->event_loop(), "Edit Property", 600, window_height (property), 0, false, parent->native_window()),
    parent_window (parent),
    property (property)
  {
    mod_list = property.modulation_list();

    FixedGrid grid;
    double yoffset = 1;

    auto hdr_pbg = new Widget (this);
    hdr_pbg->set_background_color (Color (0.4, 0.4, 0.4));
    grid.add_widget (hdr_pbg, 0, yoffset, 75, 3);

    std::string hdr_text = property.label();
    if (mod_list)
      hdr_text += " : Main Controller";
    else
      hdr_text += " : Value";
    auto hdr_prop = new Label (this, hdr_text);
    hdr_prop->set_align (TextAlign::CENTER);
    hdr_prop->set_bold (true);
    grid.add_widget (hdr_prop, 0, yoffset, 75, 3);

    yoffset += 4;

    value_label = new Label (this, property.label());

    /* INT / FLOAT property */
    line_edit = new LineEdit (this, "");
    line_edit->set_click_to_focus (true);
    slider = new Slider (this, 0);
    slider->set_int_range (property.min(), property.max());
    slider->set_int_value (property.get());
    update_line_edit_text();
    connect (line_edit->signal_text_changed, [this](std::string s) { line_edit_changed = true; });
    connect (slider->signal_int_value_changed, [this] (int i) {
      this->property.set (i);
      update_line_edit_text();
    });
    connect (line_edit->signal_return_pressed, [&]() {
      property.set_edit_str (line_edit->text());
      slider->set_int_value (property.get());
      update_line_edit_text();
    });

    if (mod_list)
      {
        auto control_combobox = main_control_view.create_combobox (this,
          property.op(),
          mod_list->main_control_type(),
          mod_list->main_control_op());

        connect (main_control_view.signal_control_changed,
          [this]()
            {
              mod_list->set_main_control_type_and_op (main_control_view.control_type(), main_control_view.op());
            });

        connect (mod_list->signal_main_control_changed, [this]()
          {
            update_layout();

            main_control_view.update_control_type_and_op (mod_list->main_control_type(), mod_list->main_control_op());
          });

        grid.add_widget (new Label (this, "Controller"), 1, yoffset, 14, 3);
        grid.add_widget (control_combobox, 11, yoffset, 17, 3);

        control_status = new ControlStatus (this, property);
        grid.add_widget (control_status, 29, yoffset, 45, 3);

        yoffset += 3;

        mod_list_hdr = new Widget (this);
        mod_list_hdr->set_background_color (Color (0.4, 0.4, 0.4));

        mod_list_label = new Label (this, "Modulation List");
        mod_list_label->set_align (TextAlign::CENTER);
        mod_list_label->set_bold (true);

        scroll_view = new ScrollView (this);
        scroll_widget = new Widget (scroll_view);

        add_mod_button = new Button (scroll_widget, "Add Modulation");
        connect (add_mod_button->signal_clicked, [this]() {
          ModulationList *mod_list = this->property.modulation_list();
          if (mod_list)
            mod_list->add_entry();
        });

        connect (mod_list->signal_size_changed, this, &PropertyViewEdit::update_modulation_widgets);
        connect (parent->signal_voice_status_changed, control_status, &ControlStatus::on_voice_status_changed);
      }

    update_layout();
    update_modulation_widgets();

    set_close_callback ([this]() { on_accept(); });

    show();
  }
  void
  update_layout()
  {
    FixedGrid grid;

    bool gui_slider_controller = !mod_list || (mod_list->main_control_type() == MorphOperator::CONTROL_GUI);
    line_edit->set_visible (gui_slider_controller);
    value_label->set_visible (gui_slider_controller);
    slider->set_visible (gui_slider_controller);

    double yoffset = 5;
    if (mod_list)
      yoffset += 3;

    if (gui_slider_controller)
      {
        grid.add_widget (value_label, 1, yoffset, 10, 3);
        grid.add_widget (slider, 11, yoffset, 50, 3);
        grid.add_widget (line_edit, 62, yoffset, 12, 3);
        yoffset += 3;
      }

    yoffset++;
    if (mod_list)
      {
        grid.add_widget (mod_list_hdr, 0, yoffset, 75, 3);
        grid.add_widget (mod_list_label, 0, yoffset, 75, 3);
        yoffset += 4;

        double h = 19;
        if (!gui_slider_controller)
          h += 4;

        grid.add_widget (scroll_view, 1, yoffset, 74, h);
        scroll_view->set_scroll_widget (scroll_widget, false, true);
      }
  }

  void
  update_modulation_widgets()
  {
    FixedGrid grid;

    auto mod_list = property.modulation_list();
    if (!mod_list)
      return;

    int yoffset = 0;
    // remove old modulation widgets created before
    for (auto w : mod_widgets)
      w->delete_later();
    mod_widgets.clear();
    for (auto cv : control_views)
      delete cv;
    control_views.clear();

    for (size_t i = 0; i < mod_list->count(); i++)
      {
        ModulationData::Entry e = (*mod_list)[i];

        // ===== remove entry button
        double xoffset = 0;
        ToolButton *tbutton = new ToolButton (scroll_widget, 'x');
        grid.add_widget (tbutton, xoffset, yoffset + 0.5, 2, 2);
        xoffset += 2.5;
        connect (tbutton->signal_clicked, [mod_list, i] () { mod_list->remove_entry (i); });

        // ===== modulation entry control op / type
        ControlView *control_view = new ControlView();
        auto control_combobox = control_view->create_combobox (scroll_widget,
          property.op(),
          e.control_type,
          e.control_op.get(),
          /* gui_slider_ok */ false);
        control_views.push_back (control_view);

        connect (control_view->signal_control_changed,
          [control_view, mod_list, i]()
            {
              ModulationData::Entry entry = (*mod_list)[i];

              entry.control_type = control_view->control_type();
              entry.control_op.set (control_view->op());

              mod_list->update_entry (i, entry);
            });

        grid.add_widget (control_combobox, xoffset, yoffset, 17, 3);
        xoffset += 18;

        // ===== unipolar / bipolar combobox

        static constexpr auto CB_UNIPOLAR_TEXT = "unipolar";
        static constexpr auto CB_BIPOLAR_TEXT = "bipolar";

        ComboBox *polarity_combobox = new ComboBox (scroll_widget);
        polarity_combobox->add_item (CB_UNIPOLAR_TEXT);
        polarity_combobox->add_item (CB_BIPOLAR_TEXT);
        polarity_combobox->set_text (e.bipolar ? CB_BIPOLAR_TEXT : CB_UNIPOLAR_TEXT);

        grid.add_widget (polarity_combobox, xoffset, yoffset, 11, 3);
        xoffset += 12;

        connect (polarity_combobox->signal_item_changed,
          [polarity_combobox, mod_list, i]()
            {
              ModulationData::Entry entry = (*mod_list)[i];
              entry.bipolar = (polarity_combobox->text() == CB_BIPOLAR_TEXT);
              mod_list->update_entry (i, entry);
            });

        // ===== mod amount slider and value label
        auto slider = new Slider (scroll_widget, (e.amount + 1) / 2);
        grid.add_widget (slider, xoffset, yoffset, 28, 3);
        xoffset += 29;

        double mod_range_ui = property.modulation_range_ui();
        auto mod_amount_model = new ParamLabelModelDouble (e.amount * mod_range_ui, -mod_range_ui, mod_range_ui, "%.3f", "%.3f");
        auto label = new ParamLabel (scroll_widget, mod_amount_model);
        grid.add_widget (label, xoffset, yoffset, 8, 3);
        xoffset += 9;

        connect (slider->signal_value_changed, [mod_amount_model, mod_list, mod_range_ui, i](double new_value) {
          ModulationData::Entry entry = (*mod_list)[i];
          entry.amount = new_value * 2 - 1;
          mod_amount_model->set_value (entry.amount * mod_range_ui);
          mod_list->update_entry (i, entry);
        });

        connect (mod_amount_model->signal_value_changed, [mod_list, mod_range_ui, i, slider](double new_value) {
          ModulationData::Entry entry = (*mod_list)[i];
          entry.amount = std::clamp (new_value / mod_range_ui, -1.0, 1.0);
          slider->set_value ((entry.amount + 1) / 2);
          mod_list->update_entry (i, entry);
        });

        mod_widgets.push_back (tbutton);
        mod_widgets.push_back (control_combobox);
        mod_widgets.push_back (polarity_combobox);
        mod_widgets.push_back (slider);
        mod_widgets.push_back (label);
        yoffset += 3;
      }
    grid.add_widget (add_mod_button, 0, yoffset, 11, 3);
    yoffset += 3;
    scroll_widget->set_height (yoffset * 8);
    scroll_view->on_widget_size_changed();
  }
  void
  update_line_edit_text()
  {
    line_edit->set_text (property.get_edit_str());

    set_keyboard_focus (line_edit, true);
    line_edit->select_all();
    line_edit_changed = false;
  }
  void
  on_accept()
  {
    if (line_edit_changed)
      property.set_edit_str (line_edit->text());

    parent_window->set_popup_window (nullptr); // close this window
  }

  void
  on_reject()
  {
    parent_window->set_popup_window (nullptr); // close this window
  }

public:
  static void
  create (MorphPlanWindow *window, Property& property)
  {
    Window *rwin = new PropertyViewEdit (window, property);

    // after this line, rename window is owned by parent window
    window->set_popup_window (rwin);
  }
};

}

#endif
