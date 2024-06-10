// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smleakdebugger.hh"
#include "smmain.hh"
#include "smdebug.hh"
#include <assert.h>
#include <glib.h>
#include <map>

using namespace SpectMorph;

using std::string;
using std::map;

static LeakDebuggerList *leak_debugger_list = nullptr;

LeakDebugger::LeakDebugger (const string& type) :
  m_type (type)
{
  if (leak_debugger_list)
    {
      leak_debugger_list->add (this);
    }
  else
    {
      g_critical ("LeakDebugger: constructor: ld_list not available, object type %s\n", m_type.c_str());
    }
}

LeakDebugger::~LeakDebugger()
{
  if (leak_debugger_list)
    {
      leak_debugger_list->del (this);
    }
  else
    {
      g_critical ("LeakDebugger: destructor: ld_list not available, object type %s\n", m_type.c_str());
    }
}

LeakDebuggerList::LeakDebuggerList()
{
  g_return_if_fail (!leak_debugger_list);
  leak_debugger_list = this;
}

LeakDebuggerList::~LeakDebuggerList()
{
  g_return_if_fail (leak_debugger_list);
  leak_debugger_list = nullptr;

  map<string, int> count_map;
  for (auto object : objects)      // ideally this should be empty (all objects deleted)
    count_map[object->type()]++;

  for (auto [type, count] : count_map)
    {
      g_printerr ("LeakDebugger (%s) => %d objects remaining\n", type.c_str(), count);
      sm_debug ("LeakDebugger (%s) => %d objects remaining\n", type.c_str(), count);
    }
}

void
LeakDebuggerList::add (LeakDebugger *leak_debugger)
{
  std::lock_guard<std::mutex> lock (mutex);

  auto result = objects.insert (leak_debugger);
  if (!result.second)
    g_critical ("LeakDebugger: invalid registration of object type %s detected\n", leak_debugger->type().c_str());
}

void
LeakDebuggerList::del (LeakDebugger *leak_debugger)
{
  std::lock_guard<std::mutex> lock (mutex);

  if (!objects.erase (leak_debugger))
    g_critical ("LeakDebugger: invalid deletion of object type %s detected\n", leak_debugger->type().c_str());
}
