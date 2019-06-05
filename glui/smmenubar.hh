// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MENUBAR_HH
#define SPECTMORPH_MENUBAR_HH

#include "smdrawutils.hh"
#include "smmath.hh"
#include "smsignal.hh"

namespace SpectMorph
{

struct MenuItem
{
  std::string text;
  Signal<> signal_clicked;
  double sx, ex, sy;
};

struct Menu
{
  std::vector<std::unique_ptr<MenuItem>> items;

  std::string title;
  double sx, ex;

  MenuItem *
  add_item (const std::string& text)
  {
    MenuItem *m;
    items.emplace_back (m = new MenuItem());
    m->text = text;
    return m;
  }
  void
  clear()
  {
    items.clear();
  }
};

struct MenuBar : public Widget
{
  std::vector<std::unique_ptr<Menu>> menus;
  int selected_menu = -1;
  bool menu_open = false;
  int selected_menu_item = -1;

  MenuBar (Widget *parent)
    : Widget (parent)
  {
  }
  Menu *
  add_menu (const std::string& title)
  {
    Menu *menu = new Menu();
    menu->title = title;
    menus.emplace_back (menu);

    update();
    return menu;
  }
  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    double space = 2;
    du.round_box (0, space, width, height - 2 * space, 1, 5, Color::null(), ThemeColor::MENU_BG);
    cairo_set_source_rgba (cr, 1, 1, 1, 1);

    double tx = 16;
    for (int item = 0; item < int (menus.size()); item++)
      {
        auto& menu_p = menus[item];

        du.bold = true;

        double start = tx;
        double tw = du.text_width (menu_p->title);
        double end = tx + tw;
        double sx = start - 16;
        double ex = end + 16;

        if (item == selected_menu)
          {
            du.round_box (sx, space, ex - sx, height - 2 * space, 1, 5, Color::null(), ThemeColor::MENU_ITEM);

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

        if (menu_open && selected_menu == item)
          {
            Menu *menu = menus[item].get();

            double max_text_width = 0;

            du.bold = false;
            for (size_t i = 0; i < menu->items.size(); i++)
              max_text_width = std::max (max_text_width, du.text_width (menu->items[i]->text));

            const double menu_height = menu->items.size() * 16 + 16;

            du.round_box (sx, height + space, max_text_width + 32, menu_height, 1, 5, ThemeColor::FRAME, ThemeColor::MENU_BG);

            double starty = height + space + 8;
            for (int i = 0; i < int (menu->items.size()); i++)
              {
                if (selected_menu_item == i)
                  {
                    du.round_box (sx + 4, starty, max_text_width + 24, 16, 1, 5, Color::null(), ThemeColor::MENU_ITEM);

                    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
                  }
                else
                  cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);

                du.text (menu->items[i]->text, sx + 16, starty, max_text_width, 16, TextAlign::LEFT);
                menu->items[i]->sx = sx;
                menu->items[i]->ex = sx + max_text_width + 32;
                menu->items[i]->sy = starty;
                starty += 16;
              }
          }
      }
  }
  bool
  clipping() override
  {
    return !menu_open;
  }
  void
  mouse_move (const MouseEvent& event) override
  {
    const int old_menu = selected_menu, old_menu_item = selected_menu_item;

    selected_menu_item = -1;

    for (size_t i = 0; i < menus.size(); i++)
      {
        if (event.x >= menus[i]->sx && event.x < menus[i]->ex && event.y >= 0 && event.y < height)
          selected_menu = i;
      }

    if (menu_open)
      {
        Menu *menu = menus[selected_menu].get();

        for (size_t i = 0; i < menu->items.size(); i++)
          {
            MenuItem *item = menu->items[i].get();

            if (event.x >= item->sx && event.x < item->ex && event.y >= item->sy && event.y < item->sy + 16)
              selected_menu_item = i;
          }
      }

    if (old_menu != selected_menu || old_menu_item != selected_menu_item)
      {
        update();
        if (menu_open)
          update_full();
      }
  }
  void
  mouse_press (const MouseEvent& event) override
  {
    if (event.button != LEFT_BUTTON)
      return;

    if (selected_menu < 0 || (menu_open && selected_menu_item < 0))
      {
        window()->set_menu_widget (nullptr);
        selected_menu = -1;
        menu_open = false;
      }
    else
      {
        window()->set_menu_widget (this);
        menu_open = true;
      }
    update_full();
  }
  void
  mouse_release (const MouseEvent& event) override
  {
    if (event.button == LEFT_BUTTON && menu_open && selected_menu_item >= 0)
      {
        MenuItem *item = menus[selected_menu]->items[selected_menu_item].get();

        item->signal_clicked();

        window()->set_menu_widget (nullptr);
        menu_open = false;
        selected_menu = -1;
        selected_menu_item = -1;

        update_full();
      }
  }
  void
  leave_event() override
  {
    selected_menu = -1;
    update();
  }
};

}

#endif


