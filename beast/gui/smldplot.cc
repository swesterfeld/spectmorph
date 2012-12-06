#include <gtkmm.h>
#include <assert.h>
#include <bse/bse.h>
#include <bse/bsemathsignal.h>
#include "smmain.hh"
#include "smzoomcontroller.hh"
#include "smmath.hh"

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

struct FreqData
{
  double freq;
  double mag;
};

struct LineData
{
  double freq1;
  double freq2;
  double mag;
};

struct FrameData
{
  size_t            index;
  vector<FreqData>  freqs;
  vector<LineData>  lines;
};

class MorphView : public Gtk::DrawingArea
{
  vector<FrameData> frame_data;
  double hzoom;
  double vzoom;
  size_t frame;
public:
  MorphView();
  bool on_expose_event (GdkEventExpose* ev);
  void load (const string& filename);
  void set_zoom (double hzoom, double vzoom);
  void set_frame (size_t frame);
  void force_redraw();
  size_t frames();
};

MorphView::MorphView()
{
  hzoom = 1;
  vzoom = 1;
  frame = 0;
}

void
MorphView::load (const string& filename)
{
  FILE *input = fopen (filename.c_str(), "r");
  assert (input);
  size_t last_index = 1 << 31;

  char buffer[1024];
  while (fgets (buffer, 1024, input))
    {
      char *id = strtok (buffer, " \n");
      if (!id)
        continue;

      char *value1 = strtok (NULL, " \n");
      if (!value1)
        continue;

      char *value2 = strtok (NULL, " \n");
      if (!value2)
        continue;

      char *value3 = strtok (NULL, " \n");
      char *id_index = strtok (id, ":");
      if (!id_index)
        continue;

      char *id_type = strtok (NULL, ": \n");
      if (!id_type)
        continue;

      FreqData f_data;
      f_data.freq = g_strtod (value1, NULL);
      f_data.mag  = g_strtod (value2, NULL);

      LineData l_data;
      l_data.freq1 = g_strtod (value1, NULL);
      l_data.freq2 = g_strtod (value2, NULL);
      if (value3)
        l_data.mag = g_strtod (value3, NULL);

      size_t index = atoi(id_index);
      if (last_index != index)
        {
          frame_data.push_back (FrameData());
          frame_data.back().index = index;
          last_index = index;
        }

      if (id_type == string ("F"))
        frame_data.back().freqs.push_back (f_data);
      else if (id_type == string ("L"))
        frame_data.back().lines.push_back (l_data);

      // printf ("INDEX(%s) TYPE(%s) VALUE(%s) VALUE(%s)\n", id_index, id_type, value1, value2);
    }
}

#if 0
static void
plot (Cairo::RefPtr<Cairo::Context> cr, vector<SpectData>& spect, int n, int width, int height)
{
  for (size_t i = 0; i < spect.size(); i++)
    {
    }
}
#endif

bool
MorphView::on_expose_event (GdkEventExpose* ev)
{
  const int width =  800 * hzoom;
  const int height = 600 * vzoom;

  set_size_request (width, height);

  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window)
    {
      Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

      cr->save();
      cr->set_source_rgb (1.0, 1.0, 1.0);   // white
      cr->paint();
      cr->restore();

      cr->set_line_width (2.0);

      // clip to the area indicated by the expose event so that we only redraw
      // the portion of the window that needs to be redrawn
      cr->rectangle (ev->area.x, ev->area.y, ev->area.width, ev->area.height);
      cr->clip();

      for (size_t i = 0; i < frame_data.size(); i++)
        {
          for (size_t j = 0; j < frame_data[i].lines.size(); j++)
            {
              const LineData& line_data = frame_data[i].lines[j];

              double db_mag = bse_db_from_factor (line_data.mag, -200);
              double color = 1.0 - CLAMP ((100 + db_mag) / 100, 0.0, 1.0);
              cr->set_source_rgb (color, color, color);

              double y1 = (1 - line_data.freq1 / 22050) * height;
              double y2 = (1 - line_data.freq2 / 22050) * height;
              double x1 = double (i - 1.0) / frame_data.size() * width;
              double x2 = double (i) / frame_data.size() * width;
              cr->move_to (x1, y1);
              cr->line_to (x2, y2);
              cr->stroke();
            }
        }
    }

  return true;
}

void
MorphView::set_zoom (double new_hzoom, double new_vzoom)
{
  hzoom = new_hzoom;
  vzoom = new_vzoom;

  force_redraw();
}

void
MorphView::set_frame (size_t new_frame)
{
  frame = new_frame;

  force_redraw();
}

void
MorphView::force_redraw()
{
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}

size_t
MorphView::frames()
{
  return frame_data.size();
}

class MorphPlotWindow : public Gtk::Window
{
  Gtk::ScrolledWindow scrolled_win;
  MorphView           morph_view;
  Gtk::VBox           vbox;
  ZoomController      zoom_controller;

  Gtk::HScale         position_scale;
  Gtk::Label          position_label;
  Gtk::HBox           position_hbox;

public:
  MorphPlotWindow (const string& filename) :
    zoom_controller (5000, 5000),
    position_scale (0, 1, 0.0001)
  {
    morph_view.load (filename);

    set_border_width (10);
    set_default_size (800, 600);

    vbox.pack_start (scrolled_win);
    vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);

    vbox.pack_start (position_hbox, Gtk::PACK_SHRINK);
    position_hbox.pack_start (position_scale);
    position_hbox.pack_start (position_label, Gtk::PACK_SHRINK);
    position_scale.set_draw_value (false);
    position_label.set_text ("frame 0");
    position_hbox.set_border_width (10);
    position_scale.signal_value_changed().connect (sigc::mem_fun (*this, &MorphPlotWindow::on_position_changed));

    scrolled_win.add (morph_view);
    add (vbox);

    zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &MorphPlotWindow::on_zoom_changed));

    show_all_children();
  }

  void
  on_zoom_changed()
  {
    morph_view.set_zoom (zoom_controller.get_hzoom(), zoom_controller.get_vzoom());
  }

  void
  on_position_changed()
  {
    size_t f = size_t (sm_round_positive (position_scale.get_value() * morph_view.frames()));
    morph_view.set_frame (CLAMP  (f, 0, morph_view.frames() - 1));
    char buffer[1024];
    sprintf (buffer, "frame %zd", f);
    position_label.set_text (buffer);
  }
};

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  Gtk::Main kit (argc, argv);

  if (argc != 2)
    {
      printf ("usage: %s <filename>\n", argv[0]);
      exit (1);
    }

  MorphPlotWindow window (argv[1]);
  Gtk::Main::run (window);
}
