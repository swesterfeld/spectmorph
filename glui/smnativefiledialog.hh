// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_NATIVE_FILE_DIALOG_HH
#define SPECTMORPH_NATIVE_FILE_DIALOG_HH

#include <string>
#include "smsignal.hh"

namespace SpectMorph
{

struct FileDialogFormats
{
  struct Format
  {
    std::string              title;
    std::vector<std::string> exts;
  };
  std::vector<Format> formats;

  FileDialogFormats()
  {
  }

  FileDialogFormats (const std::string& title, const std::string& ext)
  {
    add (title, { ext });
  }

  void
  add (const std::string& title, const std::vector<std::string>& exts)
  {
    Format format;
    format.title = title;
    format.exts  = exts;
    formats.push_back (format);
  }
};

class Window;
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
  static NativeFileDialog *create (Window *window, bool open, const std::string& title, const FileDialogFormats& formats);

  virtual void process_events() = 0;
  virtual ~NativeFileDialog() {}

  Signal<std::string> signal_file_selected;
};

}

#endif
