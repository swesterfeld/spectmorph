// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_LISTBOX_HH
#define SPECTMORPH_LISTBOX_HH

namespace SpectMorph
{

class ListBox : public Widget
{
  std::vector<std::string> items;
  int highlight_item = -1;
  int m_selected_item = -1;
  int items_per_page = 0;
  int first_item = 0;
  ScrollBar *scroll_bar = nullptr;
  const double px_starty = 8;
public:
  Signal<> signal_item_clicked;
  Signal<> signal_item_double_clicked;

  ListBox (Widget *parent)
    : Widget (parent)
  {
    scroll_bar = new ScrollBar (this, /* page_size */ 1, Orientation::VERTICAL);
    connect (scroll_bar->signal_position_changed, [=] (double pos)
      {
        first_item = lrint (pos * items.size());
        if (first_item < 0)
          first_item = 0;
        if (first_item > int (items.size()) - items_per_page)
          first_item = items.size() - items_per_page;
        update();
      });
    update_item_count();

    update_scrollbar_geometry();
    connect (signal_width_changed, this, &ListBox::update_scrollbar_geometry);
    connect (signal_height_changed, this, &ListBox::update_scrollbar_geometry);
  }
  void
  update_scrollbar_geometry()
  {
    scroll_bar->set_x (width() - 24);
    scroll_bar->set_y (8);
    scroll_bar->set_width (16);
    scroll_bar->set_height (height() - 16);
  }
  void
  update_item_count()
  {
    first_item = 0;
    scroll_bar->set_pos (0);
    const int items_on_screen = (height() - 16) / 16;
    if (items_on_screen < (int) items.size())
      {
        /* need to scroll items */
        items_per_page = items_on_screen;
        scroll_bar->set_page_size (items_per_page / double (items.size()));
        scroll_bar->set_visible (true);
      }
    else
      {
        items_per_page = items.size();
        scroll_bar->set_visible (false);
      }
    update(); // if number of items changed, we definitely need to redraw
  }
  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    double space = 2;
    du.round_box (0, space, width(), height() - 2 * space, 1, 5, ThemeColor::FRAME);

    double starty = px_starty;
    for (int i = first_item; i < first_item + items_per_page; i++)
      {
        const double box_width = scroll_bar->visible() ? width() - 28 : width() - 8;

        Color text_color (1, 1, 1);
        if (m_selected_item == i)
          {
            du.round_box (4, starty, box_width, 16, 1, 5, Color::null(), ThemeColor::MENU_ITEM);
            text_color = Color (0, 0, 0);
          }
        else if (highlight_item == i)
          du.round_box (4, starty, box_width, 16, 1, 5, Color::null(), ThemeColor::MENU_BG);

        du.set_color (text_color);
        du.text (items[i], 10, starty, box_width - 12, 16);
        starty += 16;
      }
  }
  void
  add_item (const std::string& item_text)
  {
    items.push_back (item_text);
    update_item_count();
  }
  void
  highlight_item_from_event (const MouseEvent& event)
  {
    int new_highlight_item = sm_bound<int> (0, first_item + (event.y - px_starty) / 16, items.size());

    if (new_highlight_item == (int) items.size()) /* highlight nothing */
      new_highlight_item = -1;

    if (new_highlight_item != highlight_item)
      {
        highlight_item = new_highlight_item;
        update();
      }
  }
  void
  mouse_move (const MouseEvent& event) override
  {
    highlight_item_from_event (event);
  }
  void
  leave_event() override
  {
    highlight_item = -1;
    update();
  }
  void
  mouse_press (const MouseEvent& event) override
  {
    highlight_item_from_event (event);

    if (event.button == LEFT_BUTTON)
      {
        if (event.double_click)
          {
            m_selected_item = highlight_item;
            signal_item_double_clicked();
          }
        else
          {
            m_selected_item = highlight_item;
            signal_item_clicked();
            update();
          }
      }
  }
  bool
  scroll (double dx, double dy) override
  {
    if (scroll_bar->visible())
      return scroll_bar->scroll (dx, dy);
    return false;
  }
  int
  selected_item()
  {
    return m_selected_item;
  }
  void
  clear()
  {
    items.clear();
    m_selected_item = -1;
    highlight_item = -1;
    update_item_count();
  }
};

}

#endif
