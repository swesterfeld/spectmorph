// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html
//
#ifndef SPECTMORPH_SCROLLVIEW_HH
#define SPECTMORPH_SCROLLVIEW_HH

#include "smdrawutils.hh"
#include "smscrollbar.hh"

namespace SpectMorph
{

class ScrollView : public Widget
{
  double view_width = 0;
  double view_height = 0;
  ScrollBar *h_scroll_bar = nullptr;
  ScrollBar *v_scroll_bar = nullptr;
  Widget *scroll_widget = 0;

public:
  ScrollView (Widget *parent) :
    Widget (parent)
  {
  }
  ScrollView *
  scroll_view() override
  {
    return this;
  }
  bool
  is_scroll_child (Widget *w)
  {
    /* return true for scroll_widget and all its children */
    return (w != this && w != h_scroll_bar && w != v_scroll_bar);
  }
  Rect
  child_rect()
  {
    return Rect (abs_x() + 2, abs_y() + 2, view_width - 4, view_height - 4);
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    du.round_box (0, 0, view_width, view_height, 1, 2, ThemeColor::MENU_BG);
  }
  void
  set_scroll_widget (Widget *new_scroll_widget, bool hscroll, bool vscroll)
  {
    /* cleanup old scrollbars */
    if (h_scroll_bar)
      {
        delete h_scroll_bar;
        h_scroll_bar = nullptr;
      }
    if (v_scroll_bar)
      {
        delete v_scroll_bar;
        v_scroll_bar = nullptr;
      }

    scroll_widget = new_scroll_widget;

    view_width = width;
    view_height = height;

    if (vscroll)
      view_width -= 16;
    if (hscroll)
      view_height -= 16;

    /* create new scrollbars */
    if (hscroll)
      {
        h_scroll_bar = new ScrollBar (this, 1, Orientation::HORIZONTAL);
        h_scroll_bar->x = 0;
        h_scroll_bar->y = view_height;
        h_scroll_bar->height = 16;
        h_scroll_bar->width = view_width;

        h_scroll_bar->set_scroll_factor (0.08);
        connect (h_scroll_bar->signal_position_changed, this, &ScrollView::on_scroll_bar_changed);
      }

    if (vscroll)
      {
        v_scroll_bar = new ScrollBar (this, 1, Orientation::VERTICAL);
        v_scroll_bar->x = view_width;
        v_scroll_bar->y = 0;
        v_scroll_bar->height = view_height;
        v_scroll_bar->width = 16;

        v_scroll_bar->set_scroll_factor (0.08);
        connect (v_scroll_bar->signal_position_changed, this, &ScrollView::on_scroll_bar_changed);
      }
    on_widget_size_changed();
    on_scroll_bar_changed (0);
  }
  void
  on_scroll_bar_changed (double)
  {
    scroll_widget->x = scroll_widget->y = 8;

    if (v_scroll_bar)
      scroll_widget->y -= v_scroll_bar->pos() * (scroll_widget->height + 16);
    if (h_scroll_bar)
      scroll_widget->x -= h_scroll_bar->pos() * (scroll_widget->width + 16);
    update();
  }
  bool
  scroll (double dx, double dy) override
  {
    if (v_scroll_bar)
      return v_scroll_bar->scroll (dx, dy);

    return false;
  }
  void
  on_widget_size_changed()
  {
    if (h_scroll_bar)
      {
        const double h_page_size = view_width / (scroll_widget->width + 16);

        h_scroll_bar->set_enabled (h_page_size < 1.0);
        h_scroll_bar->set_page_size (h_page_size);
      }

    if (v_scroll_bar)
      {
        const double v_page_size = view_height / (scroll_widget->height + 16);

        v_scroll_bar->set_enabled (v_page_size < 1.0);
        v_scroll_bar->set_page_size (v_page_size);
      }
  }
};

}

#endif
