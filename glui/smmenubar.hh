// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MENUBAR_HH
#define SPECTMORPH_MENUBAR_HH

#include "smdrawutils.hh"
#include "smmath.hh"

namespace SpectMorph
{

struct Menu
{
  std::string title;
  double sx, ex;
};

struct MenuBar : public Widget
{
  std::vector<std::unique_ptr<Menu>> menus;
  int selected_item = -1;
  std::unique_ptr<ComboBoxMenu> current_menu;

  MenuBar (Widget *parent)
    : Widget (parent, 0, 0, 100, 100)
  {
  }
  Menu *
  add_menu (const std::string& title)
  {
    Menu *menu = new Menu();
    menu->title = title;
    menus.emplace_back (menu);
    return menu;
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 2;
    cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 1);
    du.round_box (0, space, width, height - 2 * space, 1, 5, true);
    cairo_set_source_rgba (cr, 1, 1, 1, 1);

    double tx = 16;
    for (int item = 0; item < menus.size(); item++)
      {
        auto& menu_p = menus[item];

        double start = tx;
        double tw = du.text_width (menu_p->title);
        double end = tx + tw;
        double sx = start - 16;
        double ex = end + 16;

        if (item == selected_item)
          {
            cairo_set_source_rgba (cr, 1, 0.6, 0.0, 1);
            du.round_box (sx, space, ex - sx, height - 2 * space, 1, 5, true);

            cairo_set_source_rgba (cr, 0, 0, 0, 1); /* black text */
          }
        else
          {
            cairo_set_source_rgba (cr, 1, 1, 1, 1); /* white text */
          }

        du.text (menu_p->title, start, 0, width - 10, height);
        menu_p->sx = sx;
        tx = end + 32;
        menu_p->ex = ex;
      }
  }
  void
  motion (double x, double y) override
  {
    selected_item = -1;

    for (size_t i = 0; i < menus.size(); i++)
      {
        if (x >= menus[i]->sx && x < menus[i]->ex)
          selected_item = i;
      }
  }
  void
  mouse_press (double mx, double my) override
  {
    if (selected_item < 0)
      return;

    std::vector<ComboBoxItem> items;
    items.push_back (ComboBoxItem ("Foo"));
    items.push_back (ComboBoxItem ("Bar"));
    items.push_back (ComboBoxItem ("Bazz"));

    current_menu.reset (new ComboBoxMenu (this, menus[selected_item]->sx + x, y + height, width / 2, 100, items, ""));
    current_menu->set_done_callback ([=](const std::string& text) {
      current_menu.reset();
      window()->set_menu_widget (nullptr);
    });
    window()->set_menu_widget (this);
  }
  void
  leave_event() override
  {
    selected_item = -1;
  }
};

}

#endif


