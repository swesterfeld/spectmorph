// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smshortcut.hh"
#include "smleakdebugger.hh"
#include "smdialog.hh"
#include "smfixedgrid.hh"
#include "smlabel.hh"
#include "smbutton.hh"

using namespace SpectMorph;

using std::string;

static LeakDebugger leak_debugger ("SpectMorph::Shortcut");

/* In this implementation, shortcuts belong to a window (not a widget).
 *
 * This means:
 *  - a shortcut will get deleted if the window gets deleted
 *  - if you want to get rid of the shortcut earlier, you can delete it
 */
Shortcut::Shortcut (Window *window, PuglMod mod, uint32_t character) :
  window (window),
  mod (mod),
  mod_check (true),
  character (character)
{
  leak_debugger.add (this);

  window->add_shortcut (this);
}

Shortcut::~Shortcut()
{
  window->remove_shortcut (this);

  leak_debugger.del (this);
}


Shortcut::Shortcut (Window *window, uint32_t character) :
  window (window),
  character (character)
{
  leak_debugger.add (this);

  if (character >= 0xe000)
    {
      /* for keys like F1 or arrows, check that mod == 0 (to allow Shift+F1) */
      mod_check = true;
    }
  if (character >= uint32_t ('A') && character <= uint32_t ('Z'))
    {
      /* for upper case letters, we check that mod == Shift */
      mod = PUGL_MOD_SHIFT;
      mod_check = true;
    }
  if (character >= uint32_t ('a') && character <= uint32_t ('z'))
    {
      /* for lower case letters, we check that mod == 0 */
      mod_check = true;
    }

  window->add_shortcut (this);
}

static uint32_t
a_z_normalize (uint32_t c)
{
  if (c >= 'A' && c <= 'Z') /* Shift+A .. Shift+Z */
    return c - uint32_t ('A') + uint32_t ('a');

  if (c >= 1 && c <= 26) /* Ctrl+A .. Ctrl+Z */
    return c - 1 + uint32_t ('a');

  return c;
}

bool
Shortcut::focus_override()
{
  /* special hack to allow SPACE as shortcut, but if a textedit is active, input goes there */
  return !mod_check && character == ' ';
}

bool
Shortcut::key_press_event (const PuglEventKey& key_event)
{
  if (key_event.filter)
    {
      // printf ("filt.key_event.special=%x\n", key_event.special);
      // printf ("filt.key_event.character=%c %x\n", key_event.character, key_event.character);
      // printf ("filt.key_event.state=%d\n", key_event.state);
      /* multi key sequence -> ignore */
      return false;
    }
  // printf ("key_event.special=%x\n", key_event.special);
  // printf ("key_event.character=%c %x\n", key_event.character, key_event.character);
  // printf ("key_event.state=%d\n", key_event.state);

  const uint32_t ke_character = key_event.special ? key_event.special : key_event.character;
  if (!mod_check)
    {
      if (ke_character == character)
        {
          signal_activated();
          return true;
        }
    }
  else if (mod == key_event.state && a_z_normalize (ke_character) == a_z_normalize (character))
    {
      signal_activated();
      return true;
    }
  return false;
}

class ShortcutDebugDialog : public Dialog
{
public:
  ShortcutDebugDialog (Window *window, const string& text) :
    Dialog (window)
  {
    FixedGrid grid;

    grid.add_widget (this, 0, 0, 40, 9);

    double yoffset = 1;

    auto web_label = new Label (this, "Keyboard Shortcut:" + text);

    grid.add_widget (web_label, 10, yoffset, 40, 3);
    yoffset += 3;

    auto ok_button = new Button (this, "Ok");
    grid.add_widget (ok_button, 15, yoffset, 10, 3);
    connect (ok_button->signal_clicked, this, &Dialog::on_accept);

    window->set_keyboard_focus (this);
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


void
Shortcut::test (Window *window)
{
  static bool dialog_visible = false;
  auto gen_shortcut = [&] (const string& text, uint32_t ch, PuglMod mod = PuglMod (0)) {
    Shortcut *shortcut = mod ? new Shortcut (window, mod, ch) : new Shortcut (window, ch);
    window->connect (shortcut->signal_activated, [=] ()
        {
          if (!dialog_visible)
            {
              dialog_visible = true;

              auto dialog = new ShortcutDebugDialog (window, text.c_str());
              dialog->run ([] (bool) { dialog_visible = false; });
            }
        }
    );
  };
  for (int ch = 32; ch < 127; ch++)
    gen_shortcut (string_printf ("'%c'", ch), ch);

  for (uint32_t ch = uint32_t ('a'); ch <= uint32_t ('z'); ch++)
    {
      gen_shortcut (string_printf ("Ctrl+%c", ch), ch, PUGL_MOD_CTRL);
      gen_shortcut (string_printf ("Alt+%c", ch), ch, PUGL_MOD_ALT);
      gen_shortcut (string_printf ("Super+%c", ch), ch, PUGL_MOD_SUPER);
    }
  for (uint32_t i = 1; i <= 12; i++)
    {
      gen_shortcut (string_printf ("F%d", i), PUGL_KEY_F1 + i - 1);
      gen_shortcut (string_printf ("Shift+F%d", i), PUGL_KEY_F1 + i - 1, PUGL_MOD_SHIFT);
      gen_shortcut (string_printf ("Super+F%d", i), PUGL_KEY_F1 + i - 1, PUGL_MOD_SUPER);
    }

  struct KeyName { uint32_t key; string name; };
  std::vector<KeyName> keys = {
    { PUGL_KEY_LEFT, "Left" },
    { PUGL_KEY_UP, "Up" },
    { PUGL_KEY_RIGHT, "Right" },
    { PUGL_KEY_DOWN, "Down" },
    { PUGL_KEY_PAGE_UP, "Page Up" },
    { PUGL_KEY_PAGE_DOWN, "Page Down" },
    { PUGL_KEY_HOME, "Home" },
    { PUGL_KEY_END, "End" },
    { PUGL_KEY_INSERT, "Insert" },
  };

  for (auto k : keys)
    {
      gen_shortcut (k.name, k.key);
      gen_shortcut ("Shift+" + k.name, k.key, PUGL_MOD_SHIFT);
      gen_shortcut ("Alt+" + k.name, k.key, PUGL_MOD_ALT);
      gen_shortcut ("Ctrl+" + k.name, k.key, PUGL_MOD_CTRL);
      gen_shortcut ("Super+" + k.name, k.key, PUGL_MOD_SUPER);
    }
}
