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
public:
  double scroll_x = 0;
  double scroll_y = 0;
  double view_width = 0;
  double view_height = 0;
  ScrollBar *h_scroll_bar = nullptr;
  ScrollBar *v_scroll_bar = nullptr;
  Widget *scroll_widget = 0;

  ScrollView (Widget *parent) :
    Widget (parent)
  {
  }
  ScrollView *
  scroll_view() override
  {
    return this;
  }
  ScrollView *
  is_scroll_view() override
  {
    return this;
  }
  void
  draw (cairo_t *cr) override
  {
    DrawUtils du (cr);

    cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
    du.round_box (0, 0, view_width, view_height, 1, 2);
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
        h_scroll_bar = new ScrollBar (this, view_width / (scroll_widget->width + 16), Orientation::HORIZONTAL);
        h_scroll_bar->x = 0;
        h_scroll_bar->y = view_height;
        h_scroll_bar->height = 16;
        h_scroll_bar->width = view_width;

        connect (h_scroll_bar->signal_position_changed, this, &ScrollView::on_scroll_bar_changed);
      }

    if (vscroll)
      {
        v_scroll_bar = new ScrollBar (this, view_height / (scroll_widget->height + 16), Orientation::VERTICAL);
        v_scroll_bar->x = view_width;
        v_scroll_bar->y = 0;
        v_scroll_bar->height = view_height;
        v_scroll_bar->width = 16;

        connect (v_scroll_bar->signal_position_changed, this, &ScrollView::on_scroll_bar_changed);
      }
    on_scroll_bar_changed (0);
  }
  void
  on_scroll_bar_changed (double)
  {
    scroll_widget->x = scroll_widget->y = 8;

    if (v_scroll_bar)
      scroll_widget->y -= v_scroll_bar->pos * (scroll_widget->height + 16);
    if (h_scroll_bar)
      scroll_widget->x -= h_scroll_bar->pos * (scroll_widget->width + 16);
  }
};

}

#endif
