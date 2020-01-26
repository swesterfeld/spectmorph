// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LISTBOX_HH
#define SPECTMORPH_LISTBOX_HH

namespace SpectMorph
{

class ListBox : public Widget
{
  std::vector<std::string> items;
  int highlight_item = -1;
  int m_selected_item = -1;
  const double px_starty = 8;
public:
  Signal<> signal_item_clicked;
  Signal<> signal_item_double_clicked;

  ListBox (Widget *parent)
    : Widget (parent)
  {
  }
  void
  draw (const DrawEvent& devent) override
  {
    cairo_t *cr = devent.cr;
    DrawUtils du (cr);

    double space = 2;
    du.round_box (0, space, width, height - 2 * space, 1, 5, ThemeColor::FRAME);

    const int first_item = 0;
    const int items_per_page = std::min<size_t> ((height - 16) / 16, items.size());
    double starty = px_starty;
    for (int i = first_item; i < first_item + items_per_page; i++)
      {
        //const double box_width = scroll_bar ? width - 28 : width - 8;
        const double box_width = width - 8;

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
  }
  void
  mouse_move (const MouseEvent& event) override
  {
    const int first_item = 0;
    int new_highlight_item = sm_bound<int> (0, first_item + (event.y - px_starty) / 16, items.size() - 1);

    if (new_highlight_item != highlight_item)
      {
        highlight_item = new_highlight_item;
        update();
      }
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
  }
};

}

#endif
