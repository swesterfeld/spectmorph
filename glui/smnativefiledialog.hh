// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_NATIVE_FILE_DIALOG_HH
#define SPECTMORPH_NATIVE_FILE_DIALOG_HH

#include <string>
#include "smsignal.hh"
#include "pugl/pugl.h"

namespace SpectMorph
{

class NativeFileDialog
{
public:
  static NativeFileDialog *create (PuglNativeWindow win_id, bool open, const std::string& title, const std::string& filter);

  virtual void process_events() = 0;
  virtual ~NativeFileDialog() {}

  Signal<std::string> signal_file_selected;
};

}

#endif
