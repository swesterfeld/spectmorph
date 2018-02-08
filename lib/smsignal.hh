// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SIGNAL_HH
#define SPECTMORPH_SIGNAL_HH

#include "smutils.hh"
#include <assert.h>
#include <functional>
#include <vector>

namespace SpectMorph
{

template<class... Args>
class Signal;

struct SignalBase
{
  volatile bool m_signal_alive = true;

  virtual void disconnect (uint64 id) = 0;
  virtual
  ~SignalBase()
  {
    m_signal_alive = false;
  }
};

class SignalReceiver
{
  struct SignalSource
  {
    SignalBase *signal;
    uint64      id;
  };
  std::vector<SignalSource> m_signal_sources;
  volatile bool m_signal_receiver_alive = true;

public:
  template<class... Args, class CbFunction>
  uint64
  connect (Signal<Args...>& signal, const CbFunction& callback)
  {
    assert (m_signal_receiver_alive);

    SignalSource src { &signal, signal.connect_with_owner (this, callback) };
    m_signal_sources.push_back (src);
    return src.id;
  }
  virtual
  ~SignalReceiver()
  {
    assert (m_signal_receiver_alive);

    for (auto& signal_source : m_signal_sources)
      {
        if (signal_source.id)
          signal_source.signal->disconnect (signal_source.id);
      }
    m_signal_receiver_alive = false;
  }
  void
  dead_signal (uint64 id)
  {
    for (auto& signal_source : m_signal_sources)
      {
        if (signal_source.id == id)
          signal_source.id = 0;
      }
  }
};

template<class... Args>
struct Signal : public SignalBase
{
  typedef std::function<void (Args...)> CbFunction;

  struct SignalConnection
  {
    CbFunction      func;
    uint64          id;
    SignalReceiver *receiver;
  };
  std::vector<SignalConnection> callbacks;

  uint64
  connect_with_owner (SignalReceiver *receiver, const CbFunction& callback)
  {
    assert (m_signal_alive);

    static uint64 static_id = 1;
    callbacks.push_back ({callback, static_id, receiver});
    return static_id++;
  }
  void
  disconnect (uint64 id) override
  {
    assert (m_signal_alive);

    for (size_t i = 0; i < callbacks.size(); i++)
      {
        if (callbacks[i].id == id)
          callbacks[i].id = 0;
      }
  }
  void
  operator()(Args&&... args)
  {
    assert (m_signal_alive);

    for (auto& callback : callbacks)
      {
        if (callback.id)
          callback.func (std::forward<Args>(args)...);
      }
  }
  ~Signal()
  {
    assert (m_signal_alive);

    for (auto& callback : callbacks)
      {
        if (callback.id)
          callback.receiver->dead_signal (callback.id);
      }
  }
};

}

#endif
