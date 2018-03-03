// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smnativefiledialog.hh"

#import <Cocoa/Cocoa.h>

using std::string;

namespace SpectMorph
{

class MacFileDialog : public NativeFileDialog
{
  enum class State {
    null,
    ok,
    fail
  } state = State::null;

  std::string selected_filename;
public:
  MacFileDialog (PuglNativeWindow win_id, bool open, const std::string& filter)
  {
    if (!open)
      return;

    NSOpenPanel *panel = [NSOpenPanel openPanel];

    [panel setCanChooseFiles:YES];
    [panel setCanChooseDirectories:NO];
    [panel setAllowsMultipleSelection:NO];

    [panel beginWithCompletionHandler:^(NSInteger result) {
      if (result == NSFileHandlingPanelOKButton)
        {
          for (NSURL *url in [panel URLs])
            {
              if (![url isFileURL]) continue;

              state = State::ok;
              selected_filename = [url.path UTF8String];
              break;
            }

          if (state != State::ok)
            state = State::fail;
        }
    }];
  }
  void
  process_events() override
  {
    if (state == State::ok)
      {
        signal_file_selected (selected_filename);
        state = State::null;
      }
    else if (state == State::fail)
      {
        signal_file_selected ("");
        state = State::null;
      }
  }
};

NativeFileDialog *
NativeFileDialog::create (PuglNativeWindow win_id, bool open, const string& filter)
{
  return new MacFileDialog (win_id, open, filter);
}

}

