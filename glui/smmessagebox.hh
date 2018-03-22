// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_MESSAGEBOX_HH
#define SPECTMORPH_MESSAGEBOX_HH

#include "smdialog.hh"

namespace SpectMorph
{

class MessageBox : public Dialog
{
public:
  MessageBox (Window *window, const std::string& title, const std::string& text);

  static void critical (Widget *parent, const std::string& title, const std::string& text);
};

};

#endif
