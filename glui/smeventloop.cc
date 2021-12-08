// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
  assert (m_level == 0);

  /*
   * NOTE: on Windows OS, events can be generated and processed outside this
   * event loop, via wndProc - so all code needs to be safe for this case (as
   * well as X11/macOS which only have events in windows[i]->process_events())
   */

  signal_before_process();

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

  cleanup_null (windows);
  cleanup_null (delete_later_widgets);

  m_level--;
}

int
EventLoop::level() const
{
  return m_level;
}

bool
EventLoop::window_alive (Window *window) const
{
  /* windows that are about to be deleted are no longer considered alive */
  for (auto w : delete_later_widgets)
    if (w == window)
      return false;

  /* windows need to be on the windows list to be alive */
  for (auto w : windows)
    if (w == window)
      return true;

  return false;
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
