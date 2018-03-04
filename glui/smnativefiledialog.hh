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
protected:
  std::string
  filter2ext (const std::string& filter)
  {
    // filter="*.txt"    => ext = "txt"
    // filter="*.*"      => ext = ""
    // filter="*"        => ext = ""
    // filter="*.iso.gz" => ext = "gz"
    std::string ext;

    for (auto c : filter)
      {
        ext += c;

        if (c == '*' || c == '.')
          ext.clear();
      }
    return ext;
  }
public:
  static NativeFileDialog *create (PuglNativeWindow win_id, bool open, const std::string& title, const std::string& filter_title, const std::string& filter);

  virtual void process_events() = 0;
  virtual ~NativeFileDialog() {}

  Signal<std::string> signal_file_selected;
};

}

#endif
