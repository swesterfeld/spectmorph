// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_EXT_FILE_DIALOG_HH
#define SPECTMORPH_EXT_FILE_DIALOG_HH

#include <string>
#include "smsignal.hh"
#include "smnativefiledialog.hh"

namespace SpectMorph
{

class Window;
class ExtFileDialog : public NativeFileDialog
{
  int         child_pid;
  int         child_stdout;
  std::string selected_filename;
  bool        selected_filename_ok;

public:
  ExtFileDialog (PuglNativeWindow win_id, bool open, const std::string& filter);

  void process_events();
};

}

#endif
