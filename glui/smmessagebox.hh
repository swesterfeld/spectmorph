// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MESSAGEBOX_HH
#define SPECTMORPH_MESSAGEBOX_HH

#include "smdialog.hh"

namespace SpectMorph
{

class MessageBox : public Dialog
{
public:
  enum Buttons {
    OK = 1,
    CANCEL = 2,
    SAVE = 4,
    REVERT = 8,
  };

  MessageBox (Window *window, const std::string& title, const std::string& text, Buttons buttons);

  static void critical (Widget *parent, const std::string& title, const std::string& text);
};

inline MessageBox::Buttons operator| (MessageBox::Buttons a, MessageBox::Buttons b) { return MessageBox::Buttons (uint64_t (a) | uint64_t (b)); }
inline MessageBox::Buttons operator& (MessageBox::Buttons a, MessageBox::Buttons b) { return MessageBox::Buttons (uint64_t (a) & uint64_t (b)); }

};

#endif
