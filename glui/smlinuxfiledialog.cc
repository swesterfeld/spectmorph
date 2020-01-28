#include "smlinuxfiledialog.hh"
#include "smwindow.hh"
#include "smlineedit.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"
#include "smbutton.hh"
#include "smcombobox.hh"
#include "smlistbox.hh"
#include "smcheckbox.hh"
#include "smmessagebox.hh"
#include <glib/gstdio.h>
#include <map>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::map;

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

static bool
ends_with (const std::string& str, const std::string& suffix)
{
  /* if suffix is .wav, match foo.wav, foo.WAV, foo.Wav, ... */
  return str.size() >= suffix.size() &&
         std::equal (str.end() - suffix.size(), str.end(), suffix.begin(),
                     [] (char c1, char c2) -> bool { return tolower (c1) == tolower (c2);});
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

  struct Item
  {
    string filename;
    bool is_dir;
  };
  vector<Item> items;
  std::string current_directory;
  bool is_open_dialog = false;
  LinuxFileDialog *lfd = nullptr;
  FileDialogFormats::Format active_filter;
  map<string, FileDialogFormats::Format> filter_map;
public:
  FileDialogWindow (Window *parent_window, bool open, const string& title, const FileDialogFormats& formats, LinuxFileDialog *lfd) :
    Window (*parent_window->event_loop(), title, 320, 320, 0, false, parent_window->native_window()),
    is_open_dialog (open),
    lfd (lfd)
  {
    set_close_callback ([this, lfd]() { lfd->signal_file_selected (""); });

    FixedGrid grid;

    double yoffset = 1;
    dir_edit = new LineEdit (this, "");
    dir_edit->set_click_to_focus (true);
    grid.add_widget (dir_edit, 8, yoffset, 31, 3);

    connect (dir_edit->signal_return_pressed, [this]() {
      read_directory (dir_edit->text());
    });

    auto dir_label = new Label (this, "Directory");
    grid.add_widget (dir_label, 1, yoffset, 8, 3);
    yoffset += 3;

    list_box = new ListBox (this);
    grid.add_widget (list_box, 8, yoffset, 31, 26);
    yoffset += 26;

    connect (list_box->signal_item_clicked, [this]() {
      int i = list_box->selected_item();
      if (i >= 0 && i < int (items.size()))
        if (!items[i].is_dir)
          file_edit->set_text (items[i].filename);
    });
    connect (list_box->signal_item_double_clicked, [this,lfd]() {
      int i = list_box->selected_item();
      if (i >= 0 && i < int (items.size()))
        {
          if (items[i].is_dir)
            read_directory (current_directory + "/" + items[i].filename);
          else
            lfd->signal_file_selected (current_directory + "/" + items[i].filename);
        }
    });

    file_edit = new LineEdit (this, "");
    file_edit->set_click_to_focus (true);
    grid.add_widget (file_edit, 8, yoffset, 31, 3);

    connect (file_edit->signal_return_pressed, this, &FileDialogWindow::on_ok_clicked);
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
        filter_map[format.title] = format;
        if (first)
          {
            filter_combobox->set_text (format.title);
            active_filter = format;
            first = false;
          }
      }
    connect (filter_combobox->signal_item_changed, this, &FileDialogWindow::on_filter_changed);

    ok_button = new Button (this, open ? "Open" : "Save");
    connect (ok_button->signal_clicked, this, &FileDialogWindow::on_ok_clicked);

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
  on_ok_clicked()
  {
    // FIXME: add extension from filter if any
    lfd->signal_file_selected (current_directory + "/" + file_edit->text());
  }
  void
  on_filter_changed()
  {
    active_filter = filter_map[filter_combobox->text()];
    read_directory (current_directory);
  }
  void
  read_directory (const string& dir)
  {
    vector<string> files;
    Error error = read_dir (dir, files);
    if (error)
      {
        MessageBox::critical (this, "Error", error.message());
        /* preserve state on error */
        dir_edit->set_text (current_directory);
        return;
      }
    current_directory = dir;
    dir_edit->set_text (current_directory);
    list_box->clear();
    items.clear();

    for (auto file : files)
      {
        if (hidden_checkbox->checked() || (file.size() && file[0] != '.'))
          {
            string abs_path = dir + "/" + file;
            GStatBuf stbuf;
            if (g_stat (abs_path.c_str(), &stbuf) == 0)
              {
                Item item;
                item.filename = file;
                item.is_dir = S_ISDIR (stbuf.st_mode);

                bool filter_ok = item.is_dir;
                for (auto ext : active_filter.exts)
                  {
                    if (ext == "*" || ends_with (item.filename, ext))
                      filter_ok = true;
                  }

                if (filter_ok)
                  items.push_back (item);
              }
          }
      }
    std::sort (items.begin(), items.end(), [](Item& i1, Item& i2) {
      int d1 = i1.is_dir;
      int d2 = i2.is_dir;
      if (d1 != d2)
        return d1 > d2; // directories first

      char *k1 = g_utf8_collate_key_for_filename (i1.filename.c_str(), i1.filename.size());
      char *k2 = g_utf8_collate_key_for_filename (i2.filename.c_str(), i2.filename.size());
      string ks1 = k1, ks2 = k2;
      g_free (k1);
      g_free (k2);
      return ks1 < ks2;
    });
    for (auto item : items)
      {
        if (item.is_dir)
          list_box->add_item ("[" + item.filename + "]");
        else
          list_box->add_item (item.filename);
      }
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
