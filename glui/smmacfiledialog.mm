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

  string selected_filename;
public:
  MacFileDialog (PuglNativeWindow win_id, bool open, const string& title, const string& filter_title, const string& filter)
  {
    NSString* titleString = [[NSString alloc]
                             initWithBytes:title.c_str()
                             length:title.size()
                             encoding:NSUTF8StringEncoding];

    string extension = filter2ext (filter);

    NSMutableArray *file_types_array = [NSMutableArray new];
    [file_types_array addObject:[NSString stringWithUTF8String:extension.c_str()]];

    if (open)
      {
        NSOpenPanel *panel = [NSOpenPanel openPanel];

        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];
        [panel setTitle:titleString];
        [panel setAllowedFileTypes:file_types_array];

        [panel beginWithCompletionHandler:^(NSInteger result) {
          state = State::fail;

          if (result == NSModalResponseOK)
            {
              for (NSURL *url in [panel URLs])
                {
                  if (![url isFileURL]) continue;

                  state = State::ok;
                  selected_filename = [url.path UTF8String];
                  break;
                }
            }
        }];
      }
    else
      {
        NSSavePanel *panel = [NSSavePanel savePanel];

        [panel setTitle:titleString];
        [panel setAllowedFileTypes:file_types_array];

        [panel beginWithCompletionHandler:^(NSInteger result) {
          state = State::fail;

          if (result == NSModalResponseOK)
            {
              NSURL* url = [panel URL];
              if ([url isFileURL])
                {
                  state = State::ok;
                  selected_filename = [url.path UTF8String];
                }
            }
        }];
      }
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
NativeFileDialog::create (PuglNativeWindow win_id, bool open, const string& title, const string& filter_title, const string& filter)
{
  return new MacFileDialog (win_id, open, title, filter_title, filter);
}

}

