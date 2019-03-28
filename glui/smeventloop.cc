// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smeventloop.hh"
#include "smwindow.hh"

#include <unistd.h>

using namespace SpectMorph;

void
EventLoop::wait_event_fps()
{
  /* tradeoff between UI responsiveness and cpu usage caused by thread wakeups
   *
   * 60 fps should make the UI look smooth
   */
  const double frames_per_second = 60;

  usleep (1000 * 1000 / frames_per_second);
}

void
EventLoop::process_events()
{
  m_level++;
  for (size_t i = 0; i < windows.size(); i++) /* avoid auto here */
    {
      if (windows[i])
        windows[i]->process_events();
    }

  /* do not use auto here */
  for (size_t i = 0; i < delete_later_widgets.size(); i++)
    {
      delete delete_later_widgets[i];
      assert (!delete_later_widgets[i]);
    }

  /* FIXME: cleanup nullptr windows */
  /* FIXME: cleanup nullptr delete_later_widgets */
  m_level--;
}

int
EventLoop::level() const
{
  return m_level;
}

void
EventLoop::add_window (Window *window)
{
  windows.push_back (window);
}

void
EventLoop::remove_window (Window *window)
{
  for (auto& w : windows)
    {
      if (w == window)
        w = nullptr;
    }
  on_widget_deleted (window);
}

void
EventLoop::add_delete_later (Widget *widget)
{
  delete_later_widgets.push_back (widget);
}

void
EventLoop::on_widget_deleted (Widget *widget)
{
  for (auto& w : delete_later_widgets)
    {
      if (w == widget)
        w = nullptr;
    }
}
