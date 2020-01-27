#include "smlinuxfiledialog.hh"
#include "smwindow.hh"
#include "smlineedit.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"
#include "smbutton.hh"
#include "smcombobox.hh"
#include "smlistbox.hh"
#include "smcheckbox.hh"
#include <glib/gstdio.h>

using namespace SpectMorph;

using std::vector;
using std::string;

namespace
{

static Error
read_dir (const string& dirname, vector<string>& files)
{
  GError *gerror = nullptr;
  const char *filename;

  GDir *dir = g_dir_open (dirname.c_str(), 0, &gerror);
  if (gerror)
    {
      Error error (gerror->message);
      g_error_free (gerror);
      return error;
    }
  files.clear();
  while ((filename = g_dir_read_name (dir)))
    files.push_back (filename);
  g_dir_close (dir);

  return Error::Code::NONE;
}

class FileDialogWindow : public Window
{
  LineEdit *dir_edit;
  LineEdit *file_edit;
  ListBox *list_box;
  Button *ok_button;
  Button *cancel_button;
  ComboBox *filter_combobox;
  CheckBox *hidden_checkbox;

  vector<string> items;
  std::string current_directory;
public:
  FileDialogWindow (Window *parent_window, bool open, const string& title, const FileDialogFormats& formats, LinuxFileDialog *lfd) :
    Window (*parent_window->event_loop(), title, 320, 320, 0, false, parent_window->native_window())
  {
    set_close_callback ([this, lfd]() { lfd->signal_file_selected (""); });

    FixedGrid grid;

    double yoffset = 1;
    dir_edit = new LineEdit (this, "");
    dir_edit->set_click_to_focus (true);
    grid.add_widget (dir_edit, 8, yoffset, 31, 3);

    auto dir_label = new Label (this, "Directory");
    grid.add_widget (dir_label, 1, yoffset, 8, 3);
    yoffset += 3;

    list_box = new ListBox (this);
    grid.add_widget (list_box, 8, yoffset, 31, 26);
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

    hidden_checkbox = new CheckBox (this, "Show Hidden");
    connect (hidden_checkbox->signal_toggled, [this](bool) { read_directory (current_directory); });

    grid.add_widget (ok_button, 17, yoffset, 10, 3);
    grid.add_widget (cancel_button, 28, yoffset, 10, 3);
    grid.add_widget (hidden_checkbox, 3, yoffset + 0.5, 16, 2);

    /* put buttons left */
    auto up_button = new Button (this, "Up");
    auto home_button = new Button (this, "Home");
    auto root_button = new Button (this, "Root");

    connect (up_button->signal_pressed, [this]() {
      char *dir_name = g_path_get_dirname (current_directory.c_str());
      read_directory (dir_name);
      g_free (dir_name);
    });
    connect (home_button->signal_pressed, [this]() { read_directory (g_get_home_dir()); });
    connect (root_button->signal_pressed, [this]() { read_directory ("/"); });

    yoffset = 4;
    grid.add_widget (up_button, 1, yoffset, 6, 3);
    yoffset += 3;
    grid.add_widget (home_button, 1, yoffset, 6, 3);
    yoffset += 3;
    grid.add_widget (root_button, 1, yoffset, 6, 3);
    yoffset += 3;

    read_directory (g_get_home_dir());
  }

  void
  read_directory (const string& dir)
  {
    current_directory = dir;
    dir_edit->set_text (current_directory);
    list_box->clear();
    items.clear();

    vector<string> files;
    read_dir (dir, files);
    for (auto file : files)
      {
        if (hidden_checkbox->checked() || (file.size() && file[0] != '.'))
          {
            string abs_path = dir + "/" + file;
            GStatBuf stbuf;
            if (g_stat (abs_path.c_str(), &stbuf) == 0)
              {
                if (S_ISDIR (stbuf.st_mode))
                  list_box->add_item ("[" + file + "]");
                else
                  list_box->add_item (file);
                items.push_back (file);
              }
          }
      }
    connect (list_box->signal_item_clicked, [this]() {
      int i = list_box->selected_item();
      if (i >= 0 && i < items.size())
        file_edit->set_text (items[i]);
    });
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
