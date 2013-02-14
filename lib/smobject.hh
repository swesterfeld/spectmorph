/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPECTMORPH_OBJECT_HH
#define SPECTMORPH_OBJECT_HH

#include <birnet/birnet.hh>

#include <QObject>

namespace SpectMorph
{

class Object : public QObject
{
  Birnet::Mutex object_mutex;
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
