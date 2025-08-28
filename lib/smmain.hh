// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_MAIN_HH
#define SPECTMORPH_MAIN_HH

#include <functional>
#include <mutex>
#include <atomic>

namespace SpectMorph
{

void sm_plugin_init();
void sm_plugin_cleanup();
bool sm_init_done();
std::recursive_mutex& sm_global_data_mutex();
void sm_global_data_unlock();
bool sm_sse();
void sm_enable_sse (bool sse);
void sm_set_ui_thread();
void sm_set_dsp_thread();
bool sm_ui_thread();
bool sm_dsp_thread();
void sm_global_free_func (std::function<void()> func);

class InstEncCache;

namespace Global
{
  InstEncCache      *inst_enc_cache();
}

class Main
{
public:
  Main (int *argc_p, char ***argv_p);
  ~Main();
};

template<class T>
class Singleton
{
  std::atomic<T *> instance = nullptr;
public:
  T *ptr()
  {
    T *tmp = instance.load();
    if (!tmp)
      {
        std::lock_guard lg (sm_global_data_mutex());
        tmp = instance.load();
        if (!tmp)
          {
            tmp = new T();
            instance.store (tmp);
            sm_global_free_func ([this]
              {
                delete instance.load();
                instance.store (nullptr);
              });
          }
      }
    return tmp;
  }
};

}

#endif
