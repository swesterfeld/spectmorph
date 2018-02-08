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

struct SignalBase
{
  virtual void disconnect (uint64 id) = 0;
};

class SignalReceiver
{
  struct SignalSource
  {
    SignalBase *signal;
    uint64      id;
  };
  std::vector<SignalSource> m_signal_sources;

public:
  template<class... Args, class CbFunction>
  uint64
  connect (Signal<Args...>& signal, const CbFunction& callback)
  {
    SignalSource src { &signal, signal.connect_with_owner (callback) };
    m_signal_sources.push_back (src);
    return src.id;
  }
  virtual
  ~SignalReceiver()
  {
    for (auto& signal_source : m_signal_sources)
      {
        signal_source.signal->disconnect (signal_source.id);
      }
  }
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
