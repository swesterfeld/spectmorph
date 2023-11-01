// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smnativefiledialog.hh"
#include "smwindow.hh"

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
  MacFileDialog (PuglNativeWindow win_id, bool open, const string& title, const FileDialogFormats& formats)
  {
    NSString* titleString = [[NSString alloc]
                             initWithBytes:title.c_str()
                             length:title.size()
                             encoding:NSUTF8StringEncoding];

    NSMutableArray *file_types_array = [NSMutableArray new];

    // NSOpenPanel doesn't support multiple filters
    //  -> we use only the extensions of the first format
    //  -> to make this usable on macOS, the first format should contain all supported file extensions
    for (auto extension : formats.formats[0].exts)
      {
        [file_types_array addObject:[NSString stringWithUTF8String:extension.c_str()]];
      }

    auto display_above_parent = [win_id] (auto panel)
      {
        // display dialog above parent window
        NSView *tp_nsview = (NSView *) win_id;
        NSWindow *tp_window = [tp_nsview window];
        [tp_window addChildWindow: panel ordered: NSWindowAbove];
      };
    if (open)
      {
        NSOpenPanel *panel = [NSOpenPanel openPanel];

        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];
        [panel setTitle:titleString];
        [panel setAllowedFileTypes:file_types_array];
        display_above_parent (panel);

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
        display_above_parent (panel);

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
NativeFileDialog::create (Window *window, bool open, const string& title, const FileDialogFormats& formats)
{
  return new MacFileDialog (window->native_window(), open, title, formats);
}

}

