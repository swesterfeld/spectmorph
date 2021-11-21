// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

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
  Main main (&argc, &argv);

  FooPtr f = new Foo (1);
  FooPtr x = f;
  FooPtr y;
  y = f;
  printf ("f = %d\n", f->x());

  //Object *foo = new Foo (1);
  //foo->unref();
}
