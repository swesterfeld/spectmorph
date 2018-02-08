// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SIGNAL_HH
#define SPECTMORPH_SIGNAL_HH

#include "smutils.hh"
#include <functional>
#include <vector>

namespace SpectMorph
{

template<class... Args>
class Signal;

class SignalReceiver
{
public:
  template<class... Args, class CbFunction>
  uint64
  connect (Signal<Args...>& signal, const CbFunction& callback)
  {
    return signal.connect_with_owner (callback);
  }
  SignalReceiver()
  {
  }
  virtual
  ~SignalReceiver()
  {
  }
};

struct SignalBase
{
  virtual void disconnect (uint64 id) = 0;
};

template<class... Args>
struct Signal : public SignalBase
{
  typedef std::function<void (Args...)> CbFunction;

  struct SignalConnection
  {
    CbFunction  func;
    uint64      id;
  };
  std::vector<SignalConnection> callbacks;

  uint64
  connect_with_owner (const CbFunction& callback)
  {
    static uint64 static_id = 1;
    callbacks.push_back ({callback, static_id});
    return static_id++;
  }
  void
  disconnect (uint64 id) override
  {
    for (size_t i = 0; i < callbacks.size(); i++)
      {
        if (callbacks[i].id == id)
          callbacks[i].id = 0;
      }
  }
  void
  operator()(Args&&... args)
  {
    for (auto& callback : callbacks)
      {
        if (callback.id)
          callback.func (std::forward<Args>(args)...);
      }
  }
};

}

#endif
