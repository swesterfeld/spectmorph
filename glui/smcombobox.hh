// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_COMBOBOX_HH
#define SPECTMORPH_COMBOBOX_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

struct ComboBox;
struct ComboBoxMenu : public Widget
{
  const double px_starty = 8;

  int selected_item = 0;
  ComboBox *box;

  std::vector<std::string> items;

  ComboBoxMenu (Widget *parent, double x, double y, double weight, double height, const std::vector<std::string>& items) :
    Widget (parent, x, y, weight, height),
    items (items)
  {
    this->height = items.size() * 16 + 16;
  }
  void
  set_box (ComboBox *box)
  {
    this->box = box;
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 2;
    cairo_set_source_rgba (cr, 0.6, 0.6, 0.6, 1);
    du.round_box (0, space, width, height - 2 * space, 1, 5, true);

    double starty = px_starty;
    for (size_t i = 0; i < items.size(); i++)
      {
        if (selected_item == i)
          {
            cairo_set_source_rgba (cr, 1, 0.6, 0.0, 1);
            du.round_box (4, starty, width - 8, 16, 1, 5, true);

            cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
          }
        else
          cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);

        du.text (items[i], 10, starty, width - 10, 16);
        starty += 16;
      }
  }
  void
  motion (double x, double y) override
  {
    y -= px_starty;
    selected_item = y / 16;
    if (selected_item < 0)
      selected_item = 0;
    if (selected_item > items.size() - 1)
      selected_item = items.size() - 1;
  }
  void
  mouse_release (double x, double y) override;
};

struct ComboBox : public Widget
{
  bool mouse_down = false;
  std::unique_ptr<ComboBoxMenu> menu;
  std::string text;
  std::vector<std::string> items;

  ComboBox (Widget *parent)
    : Widget (parent, 0, 0, 100, 100)
  {
    text = "Trumpet";
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 2;
    cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 1);
    du.round_box (0, space, width, height - 2 * space, 1, 5);
    du.text (text, 10, 0, width - 10, height);
  }
  void
  mouse_press (double x, double y) override
  {
    mouse_down = true;
  }
  void
  mouse_release (double mx, double my) override
  {
    if (mouse_down)
      {
        /* click */
        menu.reset (new ComboBoxMenu (parent, x, y + height, width, 100, items));
        menu->set_box (this);
      }
    mouse_down = false;
  }
  void
  close_menu (const std::string& new_text)
  {
    text = new_text;
    menu.reset();
  }
};

void
ComboBoxMenu::mouse_release (double x, double y)
{
  box->close_menu (items[selected_item]);
}

}

#endif

