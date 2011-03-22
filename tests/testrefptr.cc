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
#include "smmain.hh"
#include <stdio.h>

using namespace SpectMorph;

class Foo : public Object
{
  int m_x;
public:
  Foo (int x) :
    m_x (x)
  {
  }
  int
  x()
  {
    return m_x;
  }
  ~Foo()
  {
    printf ("Foo (%d) destroyed\n", m_x);
  }
};

typedef RefPtr<Foo> FooPtr;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  FooPtr f = new Foo (1);
  FooPtr x = f;
  FooPtr y;
  y = f;
  printf ("f = %d\n", f->x());

  //Object *foo = new Foo (1);
  //foo->unref();
}
