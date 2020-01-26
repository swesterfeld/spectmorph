#include "smlinuxfiledialog.hh"
#include "smwindow.hh"
#include "smlineedit.hh"
#include "smframe.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"
#include "smbutton.hh"
#include "smcombobox.hh"

using namespace SpectMorph;

using std::string;

namespace
{

class FileDialogWindow : public Window
{
  LineEdit *dir_edit;
  LineEdit *file_edit;
  Frame *frame;
  Button *ok_button;
  Button *cancel_button;
  ComboBox *filter_combobox;
public:
  FileDialogWindow (Window *parent_window, bool open, const string& title, const FileDialogFormats& formats, LinuxFileDialog *lfd) :
    Window (*parent_window->event_loop(), title, 320, 320, 0, false, parent_window->native_window())
  {
    set_close_callback ([this, lfd]() { lfd->signal_file_selected (""); });

    FixedGrid grid;

    double yoffset = 1;
    dir_edit = new LineEdit (this, "/home/stefan");
    dir_edit->set_click_to_focus (true);
    grid.add_widget (dir_edit, 8, yoffset, 31, 3);

    auto dir_label = new Label (this, "Directory");
    grid.add_widget (dir_label, 1, yoffset, 8, 3);
    yoffset += 3;

    frame = new Frame (this);
    grid.add_widget (frame, 8, yoffset, 31, 26);
    yoffset += 26;

    file_edit = new LineEdit (this, "");
    file_edit->set_click_to_focus (true);
    grid.add_widget (file_edit, 8, yoffset, 31, 3);

    auto file_name = new Label (this, "Filename");
    grid.add_widget (file_name, 1, yoffset, 8, 3);
    yoffset += 3;

    grid.add_widget (new Label (this, "Filter"), 1, yoffset, 8, 3);
    filter_combobox = new ComboBox (this);
    grid.add_widget (filter_combobox, 8, yoffset, 31, 3);
    yoffset += 3;

    bool first = true;
    for (auto format : formats.formats)
      {
        filter_combobox->add_item (format.title);
        if (first)
          {
            filter_combobox->set_text (format.title);
            first = false;
          }
      }

    ok_button = new Button (this, "Ok");
    cancel_button = new Button (this, "Cancel");
    connect (cancel_button->signal_clicked, [this, lfd]() { lfd->signal_file_selected (""); });

    grid.add_widget (ok_button, 17, yoffset, 10, 3);
    grid.add_widget (cancel_button, 28, yoffset, 10, 3);

    /* put buttons left */
    auto up_button = new Button (this, "Up");
    auto home_button = new Button (this, "Home");
    auto root_button = new Button (this, "Root");

    yoffset = 4;
    grid.add_widget (up_button, 1, yoffset, 6, 3);
    yoffset += 3;
    grid.add_widget (home_button, 1, yoffset, 6, 3);
    yoffset += 3;
    grid.add_widget (root_button, 1, yoffset, 6, 3);
    yoffset += 3;
  }
};

}

LinuxFileDialog::LinuxFileDialog (Window *window, bool open, const string& title, const FileDialogFormats& formats)
{
  w.reset (new FileDialogWindow (window, open, title, formats, this));
  w->show();
}

void
LinuxFileDialog::process_events()
{
}
