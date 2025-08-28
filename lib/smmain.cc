// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmain.hh"
#include "smfft.hh"
#include "smmath.hh"
#include "smutils.hh"
#include "smconfig.hh"
#include "sminstenccache.hh"
#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <locale.h>
#include <thread>

using std::string;
using std::vector;
using std::function;
using std::min;

namespace SpectMorph
{

struct GlobalData
{
  GlobalData();
  ~GlobalData();

  LeakDebuggerList  leak_debugger_list; /* needs to be the first member to track leaks for the other members */
  InstEncCache      inst_enc_cache;

  std::thread::id   ui_thread;
  std::thread::id   dsp_thread;

  vector<function<void()>> free_functions;

  std::recursive_mutex rec_mutex;
};

static GlobalData *global_data = nullptr;

float *int_sincos_table;

static bool use_sse = true;

void
sm_enable_sse (bool sse)
{
  use_sse = sse;
}

bool
sm_sse()
{
  return use_sse;
}

static int sm_init_counter = 0;

bool
sm_init_done()
{
  return sm_init_counter > 0;
}

void
sm_plugin_init()
{
  if (sm_init_counter == 0)
    {
      assert (global_data == nullptr);
      global_data = new GlobalData();
    }
  sm_init_counter++;
  sm_debug ("sm_init_plugin: sm_init_counter = %d\n", sm_init_counter);
}

void
sm_global_free_func (function<void()> func)
{
  assert (global_data);
  global_data->free_functions.emplace_back (func);
}

void
sm_plugin_cleanup()
{
  assert (sm_init_counter > 0);

  if (sm_init_counter == 1)
    {
      delete global_data;
      global_data = nullptr;
    }
  sm_init_counter--;
  sm_debug ("sm_cleanup_plugin: sm_init_counter = %d\n", sm_init_counter);
}

GlobalData::GlobalData()
{
  /* ensure that user data dir exists */
  string user_data_dir = sm_get_user_dir (USER_DIR_DATA);
  g_mkdir_with_parents (user_data_dir.c_str(), 0775);

  /* ensure that cache dir exists */
  string cache_dir = sm_get_user_dir (USER_DIR_CACHE);
  g_mkdir_with_parents (cache_dir.c_str(), 0775);

  Config cfg;

  for (auto area : cfg.debug())
    Debug::enable (area);

  FFT::init();
  int_sincos_init();
  sm_math_init();

  sm_debug ("GlobalData instance created\n");
}

GlobalData::~GlobalData()
{
  for (auto func_it = free_functions.rbegin(); func_it != free_functions.rend(); func_it++)
    (*func_it)();
  free_functions.clear();

  FFT::cleanup();
  sm_debug ("GlobalData instance deleted\n");
}

std::recursive_mutex&
sm_global_data_mutex()
{
  return global_data->rec_mutex;
}

Main::Main (int *argc_p, char ***argv_p)
{
  /* support --debug-in-test-program option for make check */
  int&   argc = *argc_p;
  char** argv = *argv_p;

  int i = 1, j = 1;
  while (i < argc)
    {
      if (!strcmp ("--debug-in-test-program", argv[i]))
        FFT::debug_in_test_program (true);
      else
        argv[j++] = argv[i];
      i++;
    }
  argc = j;
  while (j < argc)
    argv[j++] = nullptr;

  /* internationalized string printf */
  setlocale (LC_ALL, "");
  sm_plugin_init();
}

Main::~Main()
{
  sm_plugin_cleanup();
}

InstEncCache *
Global::inst_enc_cache()
{
  g_return_val_if_fail (global_data, nullptr);
  return &global_data->inst_enc_cache;
}

void
sm_set_ui_thread()
{
  global_data->ui_thread = std::this_thread::get_id();
}

void
sm_set_dsp_thread()
{
  global_data->dsp_thread = std::this_thread::get_id();
}

bool
sm_ui_thread()
{
  return std::this_thread::get_id() == global_data->ui_thread;
}

bool
sm_dsp_thread()
{
  return std::this_thread::get_id() == global_data->dsp_thread;
}

}
