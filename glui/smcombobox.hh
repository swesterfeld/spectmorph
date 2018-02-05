// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_COMBOBOX_HH
#define SPECTMORPH_COMBOBOX_HH

#include "smdrawutils.hh"
#include "smscrollbar.hh"
#include "smmath.hh"

namespace SpectMorph
{

struct ComboBox;
struct ComboBoxItem
{
  std::string text;
  bool        headline = false;
  ComboBoxItem (const std::string& text, bool headline = false) :
    text (text),
    headline (headline)
  {
  }
};

struct ComboBoxMenu : public Widget
{
  const double px_starty = 8;

  int selected_item = 0;
  ScrollBar *scroll_bar;
  std::function<void(const std::string&)> m_done_callback;

  int items_per_page;
  int first_item;
  int release_count = 0;

  std::vector<ComboBoxItem> items;

  ComboBoxMenu (Widget *parent, double x, double y, double width, double height, const std::vector<ComboBoxItem>& items, const std::string& text) :
    Widget (parent, x, y, width, height),
    items (items)
  {
    if (items.size() > 10)
      items_per_page = 10;
    else
      items_per_page = items.size();

    first_item = 0;
    for (size_t i = 0; i < items.size(); i++)
      if (items[i].text == text)
        {
          selected_item = i;
          first_item = std::min (selected_item - items_per_page / 2, int (items.size()) - items_per_page);
          first_item = std::max (0, first_item);
        }

    this->height = items_per_page * 16 + 16;
    scroll_bar = new ScrollBar (this, double (items_per_page) / items.size());
    scroll_bar->x = x + width - 20;
    scroll_bar->y = y + 8;
    scroll_bar->width = 16;
    scroll_bar->height = items_per_page * 16;
    scroll_bar->pos = double (first_item) / items.size();

    scroll_bar->set_callback ([=] (double pos)
      {
        first_item = pos * items.size();
        if (first_item < 0)
          first_item = 0;
        if (first_item > items.size() - items_per_page)
          first_item = items.size() - items_per_page;
      });
  }
  void
  set_done_callback (const std::function<void(const std::string&)>& callback)
  {
    m_done_callback = callback;
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 2;
    cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 1);
    du.round_box (0, space, width, height - 2 * space, 1, 5, true);

    double starty = px_starty;
    for (size_t i = first_item; i < first_item + items_per_page; i++)
      {
        if (selected_item == i)
          {
            cairo_set_source_rgba (cr, 1, 0.6, 0.0, 1);
            du.round_box (4, starty, width - 28, 16, 1, 5, true);

            cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
          }
        else
          cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);

        du.bold = items[i].headline;

        TextAlign align = items[i].headline ? TextAlign::CENTER : TextAlign::LEFT;
        du.text (items[i].text, 10, starty, width - 10, 16, align);
        starty += 16;
      }
  }
  void
  motion (double x, double y) override
  {
    y -= px_starty;
    selected_item = sm_bound<int> (0, first_item + y / 16, items.size() - 1);

    int best_item = -1;
    for (size_t i = 0; i < items.size(); i++)
      {
        if (!items[i].headline)
          {
            if (best_item == -1)      // first non-headline item
              best_item = i;
            if (i <= selected_item)   // close to selected item
              best_item = i;
          }
      }
    selected_item = best_item;
  }
  void
  scroll (double dx, double dy) override
  {
    scroll_bar->scroll (dx, dy);
  }
  void
  mouse_release (double x, double y) override;
};

struct ComboBox : public Widget
{
  std::unique_ptr<ComboBoxMenu> menu;
  std::string text;
  std::vector<ComboBoxItem> items;

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
  mouse_press (double mx, double my) override
  {
    menu.reset (new ComboBoxMenu (this, x, y + height, width, 100, items, text));
    menu->set_done_callback ([=](const std::string& new_text){ close_menu (new_text); });

    window()->set_menu_widget (menu.get());
  }
  void
  close_menu (const std::string& new_text)
  {
    if (new_text != "")
      text = new_text;
    menu.reset();
  }
};

void
ComboBoxMenu::mouse_release (double mx, double my)
{
  release_count++;

  if (mx >= 0 && mx < width && my >= px_starty && my < height - px_starty)
    {
      if (m_done_callback)
        m_done_callback (items[selected_item].text);
    }
  else if (release_count == 1)
    {
      /* swallow release: this is for combobox mousedown */
    }
  else
    {
      m_done_callback ("");  /* abort */
    }
}

}

#endif

