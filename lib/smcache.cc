// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smcache.hh"

using namespace SpectMorph;

using std::string;

Cache*
Cache::the()
{
  static Cache *instance = NULL;
  if (!instance)
    instance = new Cache;

  // FIXME: cache singleton style does not work

  return instance;
}

void
Cache::unset_used()
{
  std::lock_guard<std::mutex> lg (mutex);

  for (auto& entry : cache_map)
    entry.second->used = false;
}

void
Cache::free_unused()
{
  std::lock_guard<std::mutex> lg (mutex);

  auto it = cache_map.begin();
  while (it != cache_map.end())
    {
      if (!it->second->used)
        {
          printf ("# cache delete: %s\n", it->first.c_str());
          it = cache_map.erase (it);
        }
      else
        {
          it++;
        }
    }
}

CacheEntry *
Cache::lookup (const string& key)
{
  std::lock_guard<std::mutex> lg (mutex);

  CacheEntry *ce = cache_map[key];
  if (ce) // found
    ce->used = true;

  return ce;
}

void
Cache::store (const string& key, CacheEntry *value)
{
  std::lock_guard<std::mutex> lg (mutex);

  CacheEntry* &ce = cache_map[key];
  ce = value;
  ce->used = true;
}
