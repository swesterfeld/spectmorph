// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smobject.hh"
#include <assert.h>
#include <glib.h>

using namespace SpectMorph;

Object::Object()
{
  object_ref_count = 1;
}

void
Object::ref()
{
  std::lock_guard<std::mutex> lock (object_mutex);

  assert (object_ref_count > 0);
  object_ref_count++;
}

void
Object::unref()
{
  bool destroy;
  // unlock before possible delete this
  {
    std::lock_guard<std::mutex> lock (object_mutex);
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
