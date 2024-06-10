// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_LEAK_DEBUGGER_HH
#define SPECTMORPH_LEAK_DEBUGGER_HH

#include <set>
#include <string>
#include <mutex>

namespace SpectMorph
{

class LeakDebugger
{
  std::string m_type;
public:
  LeakDebugger (const std::string& type);
  ~LeakDebugger();

  std::string type() const { return m_type; }
};

class LeakDebuggerList
{
  std::mutex                mutex;
  std::set<LeakDebugger *> objects;
public:
  LeakDebuggerList();
  ~LeakDebuggerList();

  void add (LeakDebugger *leak_debugger);
  void del (LeakDebugger *leak_debugger);
};

}

#endif
