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
  int count (const std::string& type);
};

class LeakDebugger
{
  std::mutex               mutex;
  std::map<void *, int>    ptr_map;
  std::string              type;
  std::function<void()>    cleanup_function;

  void ptr_add (void *p);
  void ptr_del (void *p);

public:
  LeakDebugger (const std::string& name, std::function<void()> cleanup_function = nullptr);
  ~LeakDebugger();

  template<class T> void add (T *instance) { ptr_add (static_cast<void *> (instance)); }
  template<class T> void del (T *instance) { ptr_del (static_cast<void *> (instance)); }
};

}

#endif
