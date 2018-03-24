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
  ScrollBar *scroll_bar = nullptr;
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
      {
        /* need scroll bar */
        items_per_page = 10;

        scroll_bar = new ScrollBar (this, double (items_per_page) / items.size(), Orientation::VERTICAL);
      }
    else
      {
        /* all items fit on screen */
        items_per_page = items.size();
      }

    first_item = 0;
    for (size_t i = 0; i < items.size(); i++)
      if (items[i].text == text)
        {
          selected_item = i;
          first_item = std::min (selected_item - items_per_page / 2, int (items.size()) - items_per_page);
          first_item = std::max (0, first_item);
        }

    this->height = items_per_page * 16 + 16;

    /* if there is not enough space below the combobox, display menu above the combobox */
    Window *win = window();
    if (win && (abs_y() + this->height + 16) > win->height)
      this->y = -this->height;

    if (scroll_bar)
      {
        scroll_bar->x = width - 20;
        scroll_bar->y = 8;
        scroll_bar->width = 16;
        scroll_bar->height = items_per_page * 16;
        scroll_bar->set_pos (double (first_item) / items.size());

        connect (scroll_bar->signal_position_changed, [=] (double pos)
          {
            first_item = pos * items.size();
            if (first_item < 0)
              first_item = 0;
            if (first_item > int (items.size()) - items_per_page)
              first_item = items.size() - items_per_page;
            update();
          });
      }
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
    du.round_box (0, space, width, height - 2 * space, 1, 5, ThemeColor::FRAME, ThemeColor::MENU_BG);

    double starty = px_starty;
    for (int i = first_item; i < first_item + items_per_page; i++)
      {
        const double box_width = scroll_bar ? width - 28 : width - 8;

        if (selected_item == i)
          {
            du.round_box (4, starty, box_width, 16, 1, 5, Color::null(), ThemeColor::MENU_ITEM);

            cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
          }
        else
          cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);

        du.bold = items[i].headline;

        TextAlign align = items[i].headline ? TextAlign::CENTER : TextAlign::LEFT;
        du.text (items[i].text, 10, starty, box_width - 12, 16, align);
        starty += 16;
      }
  }
  void
  motion (double x, double y) override
  {
    y -= px_starty;
    int new_selected_item = sm_bound<int> (0, first_item + y / 16, items.size() - 1);

    int best_item = -1;
    for (int i = 0; i < int (items.size()); i++)
      {
        if (!items[i].headline)
          {
            if (best_item == -1)      // first non-headline item
              best_item = i;
            if (i <= new_selected_item)   // close to selected item
              best_item = i;
          }
      }
    new_selected_item = best_item;
    if (new_selected_item != selected_item)
      {
        selected_item = new_selected_item;
        update();
      }
  }
  void
  scroll (double dx, double dy) override
  {
    if (scroll_bar)
      scroll_bar->scroll (dx, dy);
  }
  inline void
  mouse_release (double x, double y) override;
};

struct ComboBox : public Widget
{
protected:
  std::unique_ptr<ComboBoxMenu> menu;
  std::string m_text;
  std::vector<ComboBoxItem> items;
  bool highlight = false;

public:
  Signal<> signal_item_changed;

  void
  add_item (const ComboBoxItem& item)
  {
    items.push_back (item);
  }
  void
  add_item (const std::string& item_text)
  {
    add_item (ComboBoxItem (item_text));
  }
  void
  clear()
  {
    items.clear();
  }
  void
  set_text (const std::string& new_text)
  {
    if (m_text == new_text)
      return;

    m_text = new_text;
    update();
  }
  std::string
  text() const
  {
    return m_text;
  }
  ComboBox (Widget *parent)
    : Widget (parent)
  {
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    double space = 2;
    Color fill_color;
    if (highlight || menu)
      fill_color = ThemeColor::MENU_BG;

    Color text_color (1, 1, 1);
    Color frame_color = ThemeColor::FRAME;
    if (!recursive_enabled())
      {
        text_color = text_color.darker();
        frame_color = frame_color.darker();
      }

    du.round_box (0, space, width, height - 2 * space, 1, 5, frame_color, fill_color);

    du.set_color (text_color);
    du.text (m_text, 10, 0, width - 10, height);

    /* triangle */
    double tri_x = width - 20;
    double tri_y = height / 2 - 3;

    cairo_move_to (cr, tri_x, tri_y);
    cairo_line_to (cr, tri_x + 8, tri_y);
    cairo_line_to (cr, tri_x + 4, tri_y + 6);

    cairo_close_path (cr);
    cairo_stroke_preserve (cr);
    cairo_fill (cr);
  }
  void
  mouse_press (double mx, double my) override
  {
    menu.reset (new ComboBoxMenu (this, 0, height, width, 100, items, m_text));
    menu->set_done_callback ([=](const std::string& new_text){ close_menu (new_text); });

    window()->set_menu_widget (menu.get());
  }
  void
  enter_event() override
  {
    highlight = true;
    update();
  }
  void
  leave_event() override
  {
    highlight = false;
    update();
  }
  void
  close_menu (const std::string& new_text)
  {
    if (new_text != "")
      {
        m_text = new_text;
        signal_item_changed();
      }
    menu.reset();
    update();
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

