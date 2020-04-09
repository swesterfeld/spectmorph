// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_ENUM_VIEW_HH
#define SPECTMORPH_ENUM_VIEW_HH

namespace SpectMorph
{

class EnumView : public SignalReceiver
{
  struct Entry
  {
    int i;
    std::string s;
  };
  std::vector<Entry> entries;
public:
  void
  add_item (int i, const std::string& s)
  {
    entries.push_back (Entry { i, s });
  }
  ComboBox *
  create_combobox (Widget *parent, int initial_value, std::function<void(int)> setter)
  {
    ComboBox *combobox = new ComboBox (parent);
    for (auto entry : entries)
      {
        combobox->add_item (entry.s);
        if (initial_value == entry.i)
          combobox->set_text (entry.s);
      }
    connect (combobox->signal_item_changed, [=]()
      {
        std::string text = combobox->text();
        for (auto entry : entries)
          if (entry.s == text)
            setter (entry.i);
      });

    return combobox;
  }
};

}

#endif
