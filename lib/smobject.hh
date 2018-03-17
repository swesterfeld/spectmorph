// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_OBJECT_HH
#define SPECTMORPH_OBJECT_HH

#include <mutex>
#include "smsignal.hh"

namespace SpectMorph
{

class Object : public SignalReceiver
{
  std::mutex    object_mutex;
  unsigned int  object_ref_count;

public:
  Object();
  virtual ~Object();

  void ref();
  void unref();
};

template<class T>
class RefPtr
{
  T *ptr;

public:
  RefPtr (T* t = NULL)
  {
    ptr = t;
  }
  RefPtr (const RefPtr& other)
  {
    T *new_ptr = other.ptr;

    if (new_ptr)
      new_ptr->ref();

    ptr = new_ptr;
  }
  RefPtr&
  operator= (const RefPtr& other)
  {
    T *new_ptr = other.ptr;
    T *old_ptr = ptr;

    if (new_ptr)
      new_ptr->ref();

    ptr = new_ptr;

    if (old_ptr)
      old_ptr->unref();

    return *this;
  }
  T*
  operator->()
  {
    return ptr;
  }
  T*
  c_ptr()
  {
    return ptr;
  }
  ~RefPtr()
  {
    if (ptr)
      ptr->unref();
  }
  operator bool() const
  {
    return (ptr != 0);
  }
};

}

#endif
