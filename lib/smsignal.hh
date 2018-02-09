// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_SIGNAL_HH
#define SPECTMORPH_SIGNAL_HH

#include "smutils.hh"
#include <assert.h>
#include <functional>
#include <vector>
#include <list>

namespace SpectMorph
{

template<class... Args>
class Signal;

struct SignalBase
{
  volatile bool m_signal_alive = true;

  static uint64
  next_signal_id()
  {
    static uint64 next_id = 1;

    return next_id++;
  }
  virtual void disconnect_impl (uint64 id) = 0;
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

    SignalSource src { &signal, signal.connect_impl (this, callback) };
    m_signal_sources.push_back (src);
    return src.id;
  }
  template<class... Args, class Instance, class Method>
  uint64
  connect (Signal<Args...>& signal, Instance *instance, const Method& method)
  {
    return SignalReceiver::connect (signal, [&](Args&&... args)
      {
        (instance->*method) (std::forward<Args>(args)...);
      });
  }
  void
  disconnect (uint64 id)
  {
    for (auto& signal_source : m_signal_sources)
      {
        if (signal_source.id == id)
          {
            signal_source.signal->disconnect_impl (id);
            signal_source.id = 0;
          }
      }
  }
  virtual
  ~SignalReceiver()
  {
    assert (m_signal_receiver_alive);

    for (auto& signal_source : m_signal_sources)
      {
        if (signal_source.id)
          signal_source.signal->disconnect_impl (signal_source.id);
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
class Signal : public SignalBase
{
  typedef std::function<void (Args...)> CbFunction;

  struct SignalConnection
  {
    CbFunction      func;
    uint64          id;
    SignalReceiver *receiver;
    volatile bool   alive;

    ~SignalConnection()
    {
      alive = false;
    }
  };
  std::list<SignalConnection> callbacks;
  int block_remove_count = 0;

  void
  block_remove()
  {
    block_remove_count++;
  }
  void
  unblock_remove()
  {
    block_remove_count--;
  }
  void
  remove_unused_items()
  {
    if (!block_remove_count) // safe to remove only if not in callback
      callbacks.remove_if ([](const SignalConnection& conn) -> bool { return conn.id == 0; });
  }
public:
  uint64
  connect_impl (SignalReceiver *receiver, const CbFunction& callback)
  {
    assert (m_signal_alive);

    uint64 id = next_signal_id();
    callbacks.push_back ({callback, id, receiver, true});
    return id;
  }
  void
  disconnect_impl (uint64 id) override
  {
    assert (m_signal_alive);

    for (auto& cb : callbacks)
      {
        assert (cb.alive);

        if (cb.id == id)
          cb.id = 0;
      }
    remove_unused_items();
  }
  void
  operator()(Args&&... args)
  {
    assert (m_signal_alive);

    block_remove();  // do not free entries while we're iterating over items

    for (auto& callback : callbacks)
      {
        assert (callback.alive);

        if (callback.id)
          callback.func (std::forward<Args>(args)...);
      }

    unblock_remove();
    remove_unused_items();
  }
  ~Signal()
  {
    assert (m_signal_alive);

    for (auto& callback : callbacks)
      {
        assert (callback.alive);

        if (callback.id)
          callback.receiver->dead_signal (callback.id);
      }
  }
};

}

#endif
