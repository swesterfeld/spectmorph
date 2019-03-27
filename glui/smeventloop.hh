// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_EVENT_LOOP_HH
#define SPECTMORPH_EVENT_LOOP_HH

#include "smdrawutils.hh"

namespace SpectMorph
{

class EventLoop : public SignalReceiver
{
  std::vector<Window *> windows;
  int m_level = 0;
public:
  void wait_event_fps();
  void process_events();
  int  level() const;

  void add_window (Window *window);
  void remove_window (Window *window);
};

}

#endif
