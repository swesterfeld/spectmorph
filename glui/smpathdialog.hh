// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "smdialog.hh"
#include "smbutton.hh"

namespace SpectMorph
{

class PathDialog : public Dialog
{
  std::string get (InstallDir d)   { return sm_get_install_dir (d); }
  std::string get (UserDir d)      { return sm_get_user_dir (d); }
  std::string get (DocumentsDir d) { return sm_get_documents_dir (d); }

  template<class D> std::string
  print_dir (const char *s, D dir)
  {
    return string_printf ("%s = '%s'", s, get (dir).c_str());
  }

public:
  PathDialog (Window *window) :
    Dialog (window)
  {
    FixedGrid grid;

    double yoffset = 2;

    double tw = 0;
    auto add_label = [&] (const std::string& s)
      {
        auto label = new Label (this, s);
        tw = std::max (tw, DrawUtils::static_text_width (window, s));

        grid.add_widget (label, 2, yoffset, tw / 8 + 1, 2);
        yoffset += 2;
      };
    add_label (print_dir ("INSTALL_DIR_BIN", INSTALL_DIR_BIN));
    add_label (print_dir ("INSTALL_DIR_TEMPLATES", INSTALL_DIR_TEMPLATES));
    add_label (print_dir ("INSTALL_DIR_INSTRUMENTS", INSTALL_DIR_INSTRUMENTS));
    add_label (print_dir ("INSTALL_DIR_FONTS", INSTALL_DIR_FONTS));
    yoffset++;
    add_label (print_dir ("USER_DIR_INSTRUMENTS", USER_DIR_INSTRUMENTS));
    add_label (print_dir ("USER_DIR_CACHE", USER_DIR_CACHE));
    add_label (print_dir ("USER_DIR_DATA", USER_DIR_DATA));
    yoffset++;
    add_label (print_dir ("DOCUMENTS_DIR_INSTRUMENS", DOCUMENTS_DIR_INSTRUMENTS));
    yoffset++;
    add_label (string_printf ("%s = '%s'", "sm_get_default_plan()", sm_get_default_plan().c_str()));
    add_label (string_printf ("%s = '%s'", "sm_get_cache_dir()", sm_get_cache_dir().c_str()));
    yoffset++;

    auto ok_button = new Button (this, "Ok");
    grid.add_widget (ok_button, tw / 8 / 2 - 7.5, yoffset, 15, 3);
    connect (ok_button->signal_clicked, this, &Dialog::on_accept);
    yoffset += 3;

    window->set_keyboard_focus (this);

    grid.add_widget (this, 0, 0, tw / 8 + 4, yoffset + 1);
  }

  void
  key_press_event (const PuglEventKey& key_event) override
  {
    on_accept();
  }
  void
  mouse_press (const MouseEvent& event) override
  {
    on_accept();
  }
};

}
