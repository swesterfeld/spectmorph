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

#include "smobject.hh"
#include <assert.h>

using namespace SpectMorph;

Object::Object()
{
  object_ref_count = 1;
}

void
Object::ref()
{
  Birnet::AutoLocker lock (object_mutex);

  assert (object_ref_count > 0);
  object_ref_count++;
}

void
Object::unref()
{
  bool destroy;
  // unlock before possible delete this
  {
    Birnet::AutoLocker lock (object_mutex);
    assert (object_ref_count > 0);
    object_ref_count--;
    destroy = (object_ref_count == 0);
  }
  if (destroy)
    {
      delete this;
    }
}

Object::~Object()
{
  // need virtual destructor to be able to delete derived classes properly
  g_return_if_fail (object_ref_count == 0);
}
