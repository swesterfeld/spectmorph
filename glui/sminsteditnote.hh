// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_INST_EDIT_NOTE_HH
#define SPECTMORPH_INST_EDIT_NOTE_HH

#include "smcheckbox.hh"
#include "smparamlabel.hh"
#include "smbutton.hh"
#include "smshortcut.hh"
#include "smtoolbutton.hh"
#include "smcombobox.hh"
#include "smfixedgrid.hh"
#include "smscrollview.hh"
#include "smsynthinterface.hh"
#include "smsimplelines.hh"

namespace SpectMorph
{

class NoteWidget : public Widget
{
  int first = 12;
  int cols = 13;
  int rows = 9;
  int step = 12;

  struct NoteRect
  {
    int  note;
    Rect rect;
  };
  std::vector<NoteRect> note_rects;

  std::string
  note_to_text (int i, bool verbose = false)
  {
    std::vector<std::string> note_name { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    if (!verbose)
      {
        /* short names */
        if (note_name [i % 12][1] == '#')
          return "";
        else if (i % 12)
          return string_printf ("%s", note_name[i % 12].c_str());
      }
    /* full note name */
    return string_printf ("%s%d", note_name[i % 12].c_str(), i / 12 - 2);
  }
  int mouse_note = -1;
  int left_pressed_note = -1;
  int right_pressed_note = -1;
  Instrument     *instrument      = nullptr;
  SynthInterface *synth_interface = nullptr;
  std::vector<int> active_notes;

public:
  NoteWidget (Widget *parent, Instrument *instrument, SynthInterface *synth_interface) :
    Widget (parent),
    instrument (instrument),
    synth_interface (synth_interface)
  {
    connect (instrument->signal_samples_changed, this, &NoteWidget::on_samples_changed);
  }
  void
  draw (const DrawEvent& devent) override
  {
    DrawUtils du (devent.cr);

    du.round_box (0, 0, width(), height(), 1, 5, Color::null(), Color (0.7, 0.7, 0.7));

    auto cr = devent.cr;

    std::map<int, int> note_used = instrument->used_count();

    note_rects.clear();
    for (int xpos = 0; xpos < cols; xpos++)
      {
        for (int ypos = 0; ypos < rows; ypos++)
          {
            double x = xpos * width() / cols;
            double y = ypos * height() / rows;
            int n = first + xpos + (rows - 1 - ypos) * step;
            note_rects.push_back ({n, Rect (x, y, width() / cols, height() / rows)});
            bool white_key = !note_to_text (n).empty();
            if (white_key)
              {
                du.set_color (Color (0.0, 0.0, 0.0));
                du.text (note_to_text (n), x, y, width() / cols, height() / rows, TextAlign::CENTER);
              }
            else
              {
                cairo_set_source_rgb (cr, 0.3, 0.3, 0.3);
                cairo_rectangle (cr, x, y, width() / cols, height() / rows);
                cairo_fill (cr);
              }
            for (size_t i = 0; i < instrument->size(); i++)
              {
                Sample *sample = instrument->sample (i);
                if (sample && n == sample->midi_note())
                  {
                    double xspace = width() / cols / 15;
                    double yspace = height() / rows / 15;
                    const Rect frame_rect (x + xspace, y + yspace, width() / cols - 2 * xspace, height() / rows - 2 * yspace);

                    Color frame_color;
                    if (int (i) != instrument->selected())
                      {
                        if (white_key)
                          frame_color = Color (0.3, 0.3, 0.3);
                        else
                          frame_color = Color (0.7, 0.7, 0.7);
                        du.round_box (x + xspace, y + yspace, width() / cols - 2 * xspace, height() / rows - 2 * yspace, 3, 5, white_key ? Color (0.3, 0.3, 0.3) : Color (0.7, 0.7, 0.7));
                      }
                    else
                      {
                        frame_color = ThemeColor::MENU_ITEM;
                      }
                    /* draw red frame if one note is assigned twice */
                    if (note_used[n] > 1)
                      frame_color = Color (0.7, 0.0, 0.0);

                    du.round_box (frame_rect, 3, 5, frame_color);
                  }
              }
            bool note_playing = false;
            for (auto note : active_notes)
              if (n == note)
                note_playing = true;
            if (n == mouse_note || note_playing)
              {
                double xspace = width() / cols / 10;
                double yspace = height() / rows / 10;

                Color frame_color, fill_color, text_color;
                if (note_playing)
                  {
                    frame_color = Color::null();
                    fill_color  = ThemeColor::SLIDER;
                    text_color  = Color (0, 0, 0);
                  }
                else
                  {
                    frame_color = Color (0.8, 0.8, 0.8);
                    fill_color  = Color (0.9, 0.9, 0.9);
                    text_color  = Color (0.5, 0.0, 0.0);
                  }
                du.round_box (x + xspace, y + yspace, width() / cols - 2 * xspace, height() / rows - 2 * yspace, 1, 5, frame_color, fill_color);

                du.set_color (text_color);
                du.text (note_to_text (n, true), x, y, width() / cols, height() / rows, TextAlign::CENTER);
              }
          }
      }

    du.set_color (Color (0.4, 0.4, 0.4));
    cairo_set_line_width (cr, 1);
    for (int r = 1; r < rows; r++)
      {
        double y = r * height() / rows;
        cairo_move_to (cr, 0, y);
        cairo_line_to (cr, width(), y);
        cairo_stroke (cr);
      }
    for (int c = 1; c < cols; c++)
      {
        double x = c * width() / cols;
        cairo_move_to (cr, x, 0);
        cairo_line_to (cr, x, height());
        cairo_stroke (cr);
      }
    du.round_box (0, 0, width(), height(), 1, 5, Color (0.4, 0.4, 0.4));
  }
  void
  mouse_move (const MouseEvent& event) override
  {
    for (auto note_rect : note_rects)
      if (note_rect.rect.contains (event.x, event.y))
        {
          if (note_rect.note != mouse_note)
            {
              mouse_note = note_rect.note;
              update();
            }
        }
  }
  void
  mouse_press (const MouseEvent& event) override
  {
    if (event.double_click && event.button == LEFT_BUTTON)
      {
        Sample *sample = instrument->sample (instrument->selected());

        if (!sample)
          return;
        sample->set_midi_note (mouse_note);
      }
    else if (event.button == LEFT_BUTTON)
      {
        left_pressed_note = mouse_note;
        synth_interface->synth_inst_edit_note (left_pressed_note, true, 2);
      }
    else if (event.button == RIGHT_BUTTON)
      {
        right_pressed_note = mouse_note;
        synth_interface->synth_inst_edit_note (right_pressed_note, true, 0);
      }
    update();
  }
  void
  mouse_release (const MouseEvent& event) override
  {
    if (event.button == LEFT_BUTTON && left_pressed_note >= 0)
      {
        synth_interface->synth_inst_edit_note (left_pressed_note, false, 2);
        left_pressed_note = -1;
        update();
      }
    else if (event.button == RIGHT_BUTTON)
      {
        synth_interface->synth_inst_edit_note (right_pressed_note, false, 0);
        right_pressed_note = -1;
        update();
      }
  }
  void
  on_samples_changed()
  {
    update();
  }
  void
  set_active_notes (std::vector<int>& notes)
  {
    if (active_notes != notes)
      {
        active_notes = notes;
        update();
      }
  }
};

class InstEditNote : public Window
{
  NoteWidget *note_widget = nullptr;
public:
  InstEditNote (Window *window, Instrument *instrument, SynthInterface *synth_interface) :
    Window (*window->event_loop(), "SpectMorph - Instrument Note", 13 * 40 + 2 * 8, 9 * 40 + 6 * 8, 0, false, window->native_window())
  {
    set_close_callback ([this]() {
      signal_closed();
      delete_later();
     });

    Shortcut *play_shortcut = new Shortcut (this, ' ');
    connect (play_shortcut->signal_activated, [this]() { signal_toggle_play(); });

    note_widget = new NoteWidget (this, instrument, synth_interface);
    FixedGrid grid;
    grid.add_widget (note_widget, 1, 1, width() / 8 - 2, height() / 8 - 6);

    Label *left = new Label (this, "Left Click");
    Label *left_txt = new Label (this, "Play Reference");
    left->set_bold (true);

    Label *right = new Label (this, "Right Click");
    Label *right_txt = new Label (this, "Play Instrument");
    right->set_bold (true);

    Label *dbl = new Label (this, "Double Click");
    Label *dbl_txt = new Label (this, "Set Midi Note");
    dbl->set_bold (true);

    Label *space = new Label (this, "Space");
    Label *space_txt = new Label (this, "Play Selected Note");
    space->set_bold (true);

    grid.dx = 4;
    grid.dy = height() / 8 - 5;

    double xw = 12;
    grid.add_widget (dbl, 0, 2, xw, 3);
    grid.add_widget (dbl_txt, xw, 2, 20, 3);
    grid.add_widget (space, 0, 0, xw, 3);
    grid.add_widget (space_txt, xw, 0, 20, 3);

    grid.dx = width() / 8 / 2;
    grid.dy = height() / 8 - 5;

    grid.add_widget (new VLine (this, Color (0.6, 0.6, 0.6), 2), 0, 0, 1, 5);

    grid.dx += 4;

    xw = 13;
    grid.add_widget (left, 0, 0, xw, 3);
    grid.add_widget (left_txt, xw, 0, 20, 3);
    grid.add_widget (right, 0, 2, xw, 3);
    grid.add_widget (right_txt, xw, 2, 20, 3);

    show();
  }
  void
  set_active_notes (std::vector<int>& notes)
  {
    note_widget->set_active_notes (notes);
  }
  Signal<> signal_toggle_play;
  Signal<> signal_closed;
};

}

#endif
