// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_LEAK_DEBUGGER_HH
#define SPECTMORPH_LEAK_DEBUGGER_HH

#include <set>
#include <map>
#include <string>
#include <mutex>
#include <functional>

namespace SpectMorph
{

class LeakDebugger2
{
  std::string m_type;
public:
  LeakDebugger2 (const std::string& type);
  ~LeakDebugger2();

  std::string type() const { return m_type; }
};

class LeakDebuggerList2
{
  std::mutex                mutex;
  std::set<LeakDebugger2 *> objects;
public:
  LeakDebuggerList2();
  ~LeakDebuggerList2();

  void add (LeakDebugger2 *leak_debugger2);
  void del (LeakDebugger2 *leak_debugger2);
};

}

#endif
