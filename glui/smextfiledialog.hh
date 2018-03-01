// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_EXT_FILE_DIALOG_HH
#define SPECTMORPH_EXT_FILE_DIALOG_HH

#include <string>
#include "smsignal.hh"

namespace SpectMorph
{

class Window;
class ExtFileDialog
{
  int         child_pid;
  int         child_stdout;
  std::string selected_filename;

public:
  ExtFileDialog (Window *main_window);

  void handle_io();

  Signal<std::string> signal_file_selected;
};

}

#endif
