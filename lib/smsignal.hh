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
  }
};

class SignalReceiver
{
  struct SignalSource
  {
    SignalBase *signal;
    uint64      id;
  };
  struct SignalReceiverData
  {
    int ref_count = 1;

    SignalReceiverData *
    ref()
    {
      assert (this);
      assert (ref_count > 0);
      ref_count++;
      return this;
    }
    void
    unref (bool cleanup)
    {
      assert (this);
      assert (ref_count > 0);
      ref_count--;

      if (cleanup && ref_count == 1) /* ensure nobody is iterating over the data */
        {
          sources.remove_if ([](SignalSource& signal_source) -> bool
            {
              return signal_source.id == 0;
            });
        }
      else if (ref_count == 0)
        delete this;
    }
    std::list<SignalSource> sources;
  };
  struct SignalReceiverData *signal_receiver_data;

public:
  template<class... Args, class CbFunction>
  uint64
  connect (Signal<Args...>& signal, const CbFunction& callback)
  {
    assert (signal_receiver_data);

    SignalReceiverData *data = signal_receiver_data->ref();

    auto id = signal.connect_impl (this, callback);
    data->sources.push_back ({ &signal, id });
    data->unref (true);

    return id;
  }
  template<class... Args, class Instance, class Method>
  uint64
  connect (Signal<Args...>& signal, Instance *instance, const Method& method)
  {
    return SignalReceiver::connect (signal, [instance, method](Args&&... args)
      {
        (instance->*method) (std::forward<Args>(args)...);
      });
  }
  void
  disconnect (uint64 id)
  {
    SignalReceiverData *data = signal_receiver_data->ref();

    for (auto& signal_source : data->sources)
      {
        if (signal_source.id == id)
          {
            signal_source.signal->disconnect_impl (id);
            signal_source.id = 0;
          }
      }
    data->unref (true);
  }
  SignalReceiver() :
    signal_receiver_data (new SignalReceiverData())
  {
  }
  virtual
  ~SignalReceiver()
  {
    assert (signal_receiver_data);

    for (auto& signal_source : signal_receiver_data->sources)
      {
        if (signal_source.id)
          {
            signal_source.signal->disconnect_impl (signal_source.id);
            signal_source.id = 0;
          }
      }
    signal_receiver_data->unref (false);
    signal_receiver_data = nullptr;
  }
  void
  dead_signal (uint64 id)
  {
    SignalReceiverData *data = signal_receiver_data->ref();

    for (auto& signal_source : data->sources)
      {
        if (signal_source.id == id)
          signal_source.id = 0;
      }

    data->unref (true);
  }
};

template<class... Args>
class Signal : public SignalBase
{
  typedef std::function<void (Args...)> CbFunction;

  struct Connection
  {
    CbFunction      func;
    uint64          id;
    SignalReceiver *receiver;
  };
  struct Data
  {
    int ref_count = 1;

    Data *
    ref()
    {
      assert (this);
      assert (ref_count > 0);
      ref_count++;

      return this;
    }
    void
    unref (bool cleanup)
    {
      assert (this);
      assert (ref_count > 0);
      ref_count--;

      if (cleanup && ref_count == 1) /* ensure nobody is iterating over the data */
        {
          connections.remove_if ([](Connection& conn) -> bool
            {
              return conn.id == 0;
            });
        }
      else if (ref_count == 0)
        delete this;
    }

    std::list<Connection> connections;
  };
  Data *signal_data;
public:
  uint64
  connect_impl (SignalReceiver *receiver, const CbFunction& callback)
  {
    Data *data = signal_data->ref();
    uint64 id = next_signal_id();
    data->connections.push_back ({callback, id, receiver});
    data->unref (true);

    return id;
  }
  void
  disconnect_impl (uint64 id) override
  {
    Data *data = signal_data->ref();
    for (auto& conn : data->connections)
      {
        if (conn.id == id)
          conn.id = 0;
      }
    data->unref (true);
  }
  void
  operator()(Args... args)
  {
    Data *data = signal_data->ref();

    for (auto& conn : data->connections)
      {
        if (conn.id)
          conn.func (args...);
      }

    data->unref (true);
  }
  Signal() :
    signal_data (new Data())
  {
  }
  ~Signal()
  {
    assert (signal_data);

    for (auto& conn : signal_data->connections)
      {
        if (conn.id)
          {
            conn.receiver->dead_signal (conn.id);
            conn.id = 0;
          }
      }

    signal_data->unref (false);
    signal_data = nullptr;
  }
};

}

#endif
