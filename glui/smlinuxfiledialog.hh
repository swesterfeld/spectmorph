// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LINUXFILEDIALOG_HH
#define SPECTMORPH_LINUXFILEDIALOG_HH

#include "smnativefiledialog.hh"
#include "smwindow.hh"

namespace SpectMorph {

class LinuxFileDialog : public NativeFileDialog
{
  std::unique_ptr<Window> w;
public:
  // FIXME: merge me with NativeFileDialog create function
  LinuxFileDialog (Window *window, bool open, const std::string& title, const FileDialogFormats& formats);

  void process_events();
};

}

#endif
