// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smwindow.hh"
#include "smlineedit.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"
#include "smbutton.hh"
#include "smcombobox.hh"
#include "smlistbox.hh"
#include "smcheckbox.hh"
#include "smmessagebox.hh"
#include "smmain.hh"
#include <glib/gstdio.h>
#include <map>

using namespace SpectMorph;

using std::vector;
using std::string;
using std::map;

namespace
{

static bool
ends_with (const std::string& str, const std::string& suffix)
{
  /* if suffix is .wav, match foo.wav, foo.WAV, foo.Wav, ... */
  return str.size() >= suffix.size() &&
         std::equal (str.end() - suffix.size(), str.end(), suffix.begin(),
                     [] (char c1, char c2) -> bool { return tolower (c1) == tolower (c2);});
}

class LinuxFileDialog : public NativeFileDialog
{
  std::unique_ptr<Window> w;
public:
  LinuxFileDialog (Window *window, bool open, const std::string& title, const FileDialogFormats& formats);

  void process_events();
};

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
    string sort_key;
    bool is_dir;
  };
  vector<Item> items;
  string current_directory;
  bool is_open_dialog = false;
  LinuxFileDialog *lfd = nullptr;
  FileDialogFormats::Format active_filter;
  string default_ext;
  map<string, FileDialogFormats::Format> filter_map;

  struct Global
  {
    string      last_start_directory;
    std::mutex  mutex;
    static Global *
    the()
    {
      static Singleton<Global> singleton;
      return singleton.ptr();
    }
  };

  void
  set_last_start_directory (const string& s)
  {
    Global *g = Global::the();
    std::lock_guard lg (g->mutex);
    g->last_start_directory = s;
  }
  string
  last_start_directory()
  {
    Global *g = Global::the();
    std::lock_guard lg (g->mutex);

    return g->last_start_directory;
  }
public:
  FileDialogWindow (Window *parent_window, bool open, const string& title, const FileDialogFormats& formats, LinuxFileDialog *lfd) :
    Window (*parent_window->event_loop(), title, 480, 320, 0, false, parent_window->native_window()),
    is_open_dialog (open),
    lfd (lfd)
  {
    set_close_callback ([lfd]() { lfd->signal_file_selected (""); });

    FixedGrid grid;

    double yoffset = 1;
    dir_edit = new LineEdit (this, "");
    dir_edit->set_click_to_focus (true);
    grid.add_widget (dir_edit, 8, yoffset, 51, 3);

    connect (dir_edit->signal_return_pressed, [this]() {
      read_directory (dir_edit->text());
    });

    auto dir_label = new Label (this, "Directory");
    grid.add_widget (dir_label, 1, yoffset, 8, 3);
    yoffset += 3;

    list_box = new ListBox (this);
    grid.add_widget (list_box, 8, yoffset, 51, 26);
    yoffset += 26;

    connect (list_box->signal_item_clicked, [this]() {
      int i = list_box->selected_item();
      if (i >= 0 && i < int (items.size()))
        if (!items[i].is_dir)
          file_edit->set_text (items[i].filename);
    });
    connect (list_box->signal_item_double_clicked, [this]() {
      int i = list_box->selected_item();
      if (i >= 0 && i < int (items.size()))
        {
          if (items[i].is_dir)
            read_directory (current_directory + "/" + items[i].filename);
          else
            handle_ok (items[i].filename);
        }
    });

    file_edit = new LineEdit (this, "");
    file_edit->set_click_to_focus (true);
    grid.add_widget (file_edit, 8, yoffset, 51, 3);

    connect (file_edit->signal_return_pressed, this, &FileDialogWindow::on_ok_clicked);
    auto file_name = new Label (this, "Filename");
    grid.add_widget (file_name, 1, yoffset, 8, 3);
    yoffset += 3;

    grid.add_widget (new Label (this, "Filter"), 1, yoffset, 8, 3);
    filter_combobox = new ComboBox (this);
    grid.add_widget (filter_combobox, 8, yoffset, 51, 3);
    yoffset += 3;

    for (size_t i = 0; i < formats.formats.size(); i++)
      {
        const auto& format = formats.formats[i];

        filter_combobox->add_item (format.title);
        filter_map[format.title] = format;
        if (i == 0)
          {
            filter_combobox->set_text (format.title);
            active_filter = format;

            // NOTE: if the current filter has an extension, this extension will
            // be used, rather than the default extension, so in most cases the
            // actual value given here is ignored
            if (format.exts.size())
              default_ext = format.exts[0];
          }
      }

    connect (filter_combobox->signal_item_changed, this, &FileDialogWindow::on_filter_changed);

    ok_button = new Button (this, open ? "Open" : "Save");
    connect (ok_button->signal_clicked, this, &FileDialogWindow::on_ok_clicked);

    cancel_button = new Button (this, "Cancel");
    connect (cancel_button->signal_clicked, [lfd]() { lfd->signal_file_selected (""); });

    hidden_checkbox = new CheckBox (this, "Show Hidden");
    connect (hidden_checkbox->signal_toggled, [this](bool) { read_directory (current_directory); });

    grid.add_widget (ok_button, 37, yoffset, 10, 3);
    grid.add_widget (cancel_button, 48, yoffset, 10, 3);
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

    if (last_start_directory() != "" && can_read_dir (last_start_directory()))
      read_directory (last_start_directory());
    else
      read_directory (g_get_home_dir());
  }
  bool
  can_read_dir (const string& dirname)
  {
    /* simple check if directory was deleted */
    vector<string> files;
    Error error = read_dir (dirname, files);
    return !error;
  }
  void
  on_ok_clicked()
  {
    if (file_edit->text() != "")
      {
        /* open file */
        handle_ok (file_edit->text());
      }
    else
      {
        /* open selected dir (if any) */
        int i = list_box->selected_item();
        if (i >= 0 && i < int (items.size()))
          {
            if (items[i].is_dir)
              read_directory (current_directory + "/" + items[i].filename);
          }
      }
  }
  void
  handle_ok (const string& filename)
  {
    string path = current_directory + "/" + filename;

    /* open dialog is easy */
    if (is_open_dialog)
      {
        set_last_start_directory (current_directory);
        lfd->signal_file_selected (path);
        return;
      }

    /* save dialog */
    if (!g_file_test (path.c_str(), G_FILE_TEST_EXISTS))
      {
        /* append extension if necessary */
        string need_ext = default_ext;
        if (active_filter.exts.size() == 1 && active_filter.exts[0] != "*")
          need_ext = active_filter.exts[0];

        if (need_ext != "" && !ends_with (path, "." + need_ext))
          path += "." + need_ext;
      }

    if (g_file_test (path.c_str(), G_FILE_TEST_EXISTS))
      {
        /* confirm overwrite */
        char *fn = g_path_get_basename (path.c_str());
        string message = string ("File '") + fn + "' already exists.\n\nDo you wish to overwrite it?";
        g_free (fn);

        auto confirm_box = new MessageBox (window(), "Overwrite File?", message, MessageBox::SAVE | MessageBox::CANCEL);
        confirm_box->run ([this, path](bool save_changes)
          {
            if (save_changes)
              {
                set_last_start_directory (current_directory);
                lfd->signal_file_selected (path);
              }
          });
      }
    else
      {
        set_last_start_directory (current_directory);
        lfd->signal_file_selected (path);
      }
  }
  void
  on_filter_changed()
  {
    active_filter = filter_map[filter_combobox->text()];
    read_directory (current_directory);
  }
  string
  canonicalize (const string& path)
  {
    string result = path;

    char *real_path = realpath (path.c_str(), nullptr);
    if (real_path)
      result = real_path;
    free (real_path);

    return result;
  }
  void
  read_directory (const string& new_dir)
  {
    string dir = canonicalize (new_dir);
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
                    if (ext == "*" || ends_with (item.filename, "." + ext))
                      filter_ok = true;
                  }

                if (filter_ok)
                  items.push_back (item);
              }
          }
      }
    if (dir != "/")
      {
        Item parent_item;
        parent_item.filename = "..";
        parent_item.is_dir = true;
        items.push_back (parent_item);
      }

    for (auto& item : items)
      {
        char *utf8_valid_name = g_utf8_make_valid (item.filename.c_str(), -1);
        char *name_nocase = g_utf8_casefold (utf8_valid_name, -1);
        char *sort_key = g_utf8_collate_key_for_filename (name_nocase, -1);
        item.sort_key = sort_key;
        g_free (sort_key);
        g_free (name_nocase);
        g_free (utf8_valid_name);
      }

    std::sort (items.begin(), items.end(), [](Item& i1, Item& i2) {
      int d1 = i1.is_dir;
      int d2 = i2.is_dir;
      if (d1 != d2)
        return d1 > d2; // directories first

      return i1.sort_key < i2.sort_key;
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

NativeFileDialog *
NativeFileDialog::create (Window *window, bool open, const string& title, const FileDialogFormats& formats)
{
  return new LinuxFileDialog (window, open, title, formats);
}
