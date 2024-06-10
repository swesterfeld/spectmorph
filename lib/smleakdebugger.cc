// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smleakdebugger.hh"
#include "smmain.hh"
#include "smdebug.hh"
#include <assert.h>
#include <glib.h>

#define DEBUG (1)

using namespace SpectMorph;

using std::string;
using std::map;

static LeakDebuggerList2 *leak_debugger_list2 = nullptr;

LeakDebugger2::LeakDebugger2 (const string& type) :
  m_type (type)
{
  if (leak_debugger_list2)
    {
      leak_debugger_list2->add (this);
    }
  else
    {
      g_critical ("LeakDebugger2: constructor: ld_list not available, object type %s\n", m_type.c_str());
    }
}

LeakDebugger2::~LeakDebugger2()
{
  if (leak_debugger_list2)
    {
      leak_debugger_list2->del (this);
    }
  else
    {
      g_critical ("LeakDebugger2: destructor: ld_list not available, object type %s\n", m_type.c_str());
    }
}

LeakDebuggerList2::LeakDebuggerList2()
{
  g_return_if_fail (!leak_debugger_list2);
  leak_debugger_list2 = this;
}

LeakDebuggerList2::~LeakDebuggerList2()
{
  g_return_if_fail (leak_debugger_list2);
  leak_debugger_list2 = nullptr;

  map<string, int> count_map;
  for (auto object : objects)      // ideally this should be empty (all objects deleted)
    count_map[object->type()]++;

  for (auto [type, count] : count_map)
    {
      g_printerr ("LeakDebugger2 (%s) => %d objects remaining\n", type.c_str(), count);
      sm_debug ("LeakDebugger2 (%s) => %d objects remaining\n", type.c_str(), count);
    }
}

void
LeakDebuggerList2::add (LeakDebugger2 *leak_debugger2)
{
  std::lock_guard<std::mutex> lock (mutex);

  auto result = objects.insert (leak_debugger2);
  if (!result.second)
    g_critical ("LeakDebugger2: invalid registration of object type %s detected\n", leak_debugger2->type().c_str());
}

void
LeakDebuggerList2::del (LeakDebugger2 *leak_debugger2)
{
  std::lock_guard<std::mutex> lock (mutex);

  if (!objects.erase (leak_debugger2))
    g_critical ("LeakDebugger2: invalid deletion of object type %s detected\n", leak_debugger2->type().c_str());
}
