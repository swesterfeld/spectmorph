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

#include "smleakdebugger.hh"
#include <assert.h>

#define DEBUG (1)

using namespace SpectMorph;

using std::string;
using std::map;

void
LeakDebugger::ptr_add (void *p)
{
  if (DEBUG)
    {
      Birnet::AutoLocker lock (mutex);

      if (ptr_map[p] != 0)
        g_critical ("LeakDebugger: invalid registration of object type %s detected; ptr_map[p] is %d\n",
                    type.c_str(), ptr_map[p]);

      ptr_map[p]++;
    }
}

void
LeakDebugger::ptr_del (void *p)
{
  if (DEBUG)
    {
      Birnet::AutoLocker lock (mutex);

      if (ptr_map[p] != 1)
        g_critical ("LeakDebugger: invalid deletion of object type %s detected; ptr_map[p] is %d\n",
                    type.c_str(), ptr_map[p]);

      ptr_map[p]--;
    }
}

LeakDebugger::LeakDebugger (const string& name) :
  type (name)
{
}

LeakDebugger::~LeakDebugger()
{
  if (DEBUG)
    {
      int alive = 0;

      for (map<void *, int>::iterator pi = ptr_map.begin(); pi != ptr_map.end(); pi++)
        {
          if (pi->second != 0)
            {
              assert (pi->second == 1);
              alive++;
            }
        }
      if (alive)
        {
          g_printerr ("LeakDebugger (%s) => %d objects remaining\n", type.c_str(), alive);
        }
    }
}
