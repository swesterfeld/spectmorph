// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_TIMER_HH
#define SPECTMORPH_TIMER_HH

#include "smwindow.hh"

namespace SpectMorph
{

class Timer
{
  Window *window = nullptr;
  int     interval_ms = -1;
  double  timestamp   = -1;
  double  running_ms  = 0;
public:
  Timer (Window *window);
  ~Timer();

  void start (int ms);
  void stop();
  bool active();

  void process_events();

  Signal<> signal_timeout;
};

}

#endif
