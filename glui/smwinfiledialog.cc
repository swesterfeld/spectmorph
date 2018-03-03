// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smnativefiledialog.hh"
#include "pugl/pugl.h"
#include <mutex>
#include <memory>
#include <thread>
#include "Windows.h"

using std::string;

namespace SpectMorph
{

class WinFileDialog : public NativeFileDialog
{
  enum class State {
    null,
    running,
    ok,
    fail
  } state = State::null;

  std::unique_ptr<std::thread> dialog_thread;
  std::mutex mutex;

  string selected_filename;
public:
  WinFileDialog (PuglNativeWindow win_id, bool open, const std::string& filter)
  {
    state = State::running;
    dialog_thread.reset (new std::thread ([=]() { thread_run (win_id, open, filter); }));
  }
  ~WinFileDialog()
  {
    if (dialog_thread)
      dialog_thread->join();
  }
  void
  thread_run (PuglNativeWindow win_id, bool open, const string& filter)
  { 
    OPENFILENAME ofn;
    char filename[1024] = "";

    ZeroMemory (&ofn, sizeof (OPENFILENAME));
    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.lpstrFile = filename;
    ofn.nMaxFile = 1024;
    ofn.lpstrTitle = "!FIXME: window title";
    ofn.lpstrFilter = "All\0*.*\0";
    ofn.nFilterIndex = 0;
    ofn.lpstrInitialDir = 0;
    ofn.Flags = OFN_ENABLESIZING | OFN_NONETWORKBUTTON | OFN_HIDEREADONLY | OFN_READONLY;
    if (open)
      ofn.Flags |= OFN_FILEMUSTEXIST;
    ofn.hwndOwner = (HWND) win_id; // modal

    auto fn_result = open ? GetOpenFileName (&ofn) : GetSaveFileName (&ofn);

    std::lock_guard<std::mutex> lg (mutex);
    if (fn_result)
      {
        selected_filename = filename;
        state = State::ok;
      }
    else
      {
        state = State::fail;
      }
  }
  void
  process_events()
  {
    std::lock_guard<std::mutex> lg (mutex);

    if (state == State::ok)
      {
        signal_file_selected (selected_filename);
        state = State::null;
      }
    if (state == State::fail)
      {
        signal_file_selected ("");
        state = State::null;
      }
  }
};

NativeFileDialog *
NativeFileDialog::create (PuglNativeWindow win_id, bool open, const string& filter)
{
  return new WinFileDialog (win_id, open, filter);
}

}