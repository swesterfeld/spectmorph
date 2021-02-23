// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PROPERTY_VIEW_EDIT_HH
#define SPECTMORPH_PROPERTY_VIEW_EDIT_HH

#include "smwindow.hh"
#include "smbutton.hh"
#include "smlineedit.hh"
#include "smcontrolview.hh"

namespace SpectMorph
{

class PropertyViewEdit : public Window
{
protected:
  Window         *parent_window;
  MorphOperator  *op;
  Property&       property;

  Button         *ok_button;
  Button         *add_mod_button;
  LineEdit       *line_edit;
  bool            line_edit_changed = false;

  std::vector<ControlView *> control_views;
  std::vector<Widget *> mod_widgets;

  PropertyViewEdit (Window *parent, MorphOperator *op, Property& property) :
    Window (*parent->event_loop(), "Edit Property", 320, 320, 0, false, parent->native_window()),
    parent_window (parent),
    op (op),
    property (property)
  {
    FixedGrid grid;

    double yoffset = 2;
    grid.add_widget (new Label (this, property.label()), 1, yoffset, 30, 3);
    if (property.type() == Property::Type::FLOAT)
      {
        grid.add_widget (new Label (this, property.label()), 1, yoffset, 30, 3);
        line_edit = new LineEdit (this, string_locale_printf ("%.3f", property.get_float()));
        line_edit->select_all();
        grid.add_widget (line_edit, 10, yoffset, 20, 3);
        line_edit->set_click_to_focus (true);
        connect (line_edit->signal_text_changed, [this](std::string s) { line_edit_changed = true; });
        set_keyboard_focus (line_edit, true);
      }

    yoffset += 3;


    add_mod_button = new Button (this, "Add Modulation");
    connect (add_mod_button->signal_clicked, [this]() {
      ModulationList *mod_list = this->property.modulation_list();
      if (mod_list)
        {
          mod_list->add_entry();
          update_modulation_widgets();
        }
    });

    ok_button = new Button (this, "Close");

    update_modulation_widgets();

    connect (line_edit->signal_return_pressed,  [=]() {
      if (ok_button->enabled())
        on_accept();
    });
    connect (ok_button->signal_clicked,     this, &PropertyViewEdit::on_accept);

    set_close_callback ([this]() { on_accept(); });

    show();
  }

  void
  update_modulation_widgets()
  {
    FixedGrid grid;

    auto mod_list = property.modulation_list();
    if (!mod_list)
      return;

    int yoffset = 10;
    // remove old modulation widgets created before
    for (auto w : mod_widgets)
      w->delete_later();
    mod_widgets.clear();
    for (auto cv : control_views)
      delete cv;
    control_views.clear();

    for (size_t i = 0; i < mod_list->size(); i++)
      {
        ModulationList::Entry e = (*mod_list)[i];

        ControlView *control_view = new ControlView();
        auto control_combobox = control_view->create_combobox (this,
          op,
          e.control_type,
          e.control_op.get());
        control_views.push_back (control_view);

        connect (control_view->signal_control_changed,
          [control_view, mod_list, i]()
            {
              ModulationList::Entry& update_entry = (*mod_list)[i];

              update_entry.control_type = control_view->control_type();
              update_entry.control_op.set (control_view->op());
            });

        grid.add_widget (control_combobox, 1, yoffset, 15, 3);
        auto slider = new Slider (this, (e.mod_amount + 1) / 2);
        grid.add_widget (slider, 20, yoffset, 15, 3);
        auto label = new Label (this, string_printf ("%.3f", e.mod_amount));
        grid.add_widget (label, 32, yoffset, 15, 3);
        ToolButton *tbutton = new ToolButton (this, 'x');
        grid.add_widget (tbutton, 37.5, yoffset + 0.5, 2, 2);
        connect (tbutton->signal_clicked,
          [mod_list, i, this] ()
            {
              mod_list->remove_entry (i);
              update_modulation_widgets();
            });
        connect (slider->signal_value_changed, [label, slider, mod_list, i](double new_value) {
          ModulationList::Entry& update_entry = (*mod_list)[i];
          update_entry.mod_amount = new_value * 2 - 1;
          label->set_text (string_printf ("%.3f", update_entry.mod_amount));
        });

        mod_widgets.push_back (control_combobox);
        mod_widgets.push_back (label);
        mod_widgets.push_back (slider);
        mod_widgets.push_back (tbutton);
        yoffset += 3;
      }
    grid.add_widget (add_mod_button, 7, yoffset, 31, 3);
    yoffset += 3;
    grid.add_widget (ok_button, 17, yoffset, 10, 3);
  }
  void
  on_accept()
  {
    if (line_edit_changed)
      property.set_float (atof (line_edit->text().c_str()));

    parent_window->set_popup_window (nullptr); // close this window
  }

  void
  on_reject()
  {
    parent_window->set_popup_window (nullptr); // close this window
  }

public:
  static void
  create (Window *window, MorphOperator *op, Property& property)
  {
    Window *rwin = new PropertyViewEdit (window, op, property);

    // after this line, rename window is owned by parent window
    window->set_popup_window (rwin);
  }
};

}

#endif
