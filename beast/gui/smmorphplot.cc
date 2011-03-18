#include <gtkmm.h>
#include <assert.h>
#include <map>
#include <bse/bse.h>
#include <bse/bsemathsignal.h>
#include "smmain.hh"
#include "smzoomcontroller.hh"

using namespace SpectMorph;

using std::string;
using std::vector;
using std::max;

struct SpectData
{
  double freq;
  double mag;
};

struct LineData
{
  double freq1;
  double freq2;
};

struct FrameData
{
  vector<SpectData> a;
  vector<SpectData> b;
  vector<LineData>  lines;
};

class MorphView : public Gtk::DrawingArea
{
  std::map<size_t, FrameData> frame_data;
  double hzoom;
  double vzoom;
public:
  MorphView();
  bool on_expose_event (GdkEventExpose* ev);
  void load (const string& filename);
  void set_zoom (double hzoom, double vzoom);
  void force_redraw();
};

MorphView::MorphView()
{
  hzoom = 1;
  vzoom = 1;
}

void
MorphView::load (const string& filename)
{
  FILE *input = fopen (filename.c_str(), "r");
  assert (input);

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

      char *id_index = strtok (id, ":");
      if (!id_index)
        continue;

      char *id_type = strtok (NULL, ": \n");
      if (!id_type)
        continue;

      SpectData s_data;
      s_data.freq = g_strtod (value1, NULL);
      s_data.mag  = g_strtod (value2, NULL);

      LineData l_data;
      l_data.freq1 = g_strtod (value1, NULL);
      l_data.freq2 = g_strtod (value2, NULL);

      if (id_type == string ("A"))
        frame_data[atoi(id_index)].a.push_back (s_data);
      else if (id_type == string ("B"))
        frame_data[atoi(id_index)].b.push_back (s_data);
      else if (id_type == string ("L"))
        frame_data[atoi(id_index)].lines.push_back (l_data);

      //printf ("INDEX(%s) TYPE(%s) VALUE(%s) VALUE(%s)\n", id_index, id_type, value1, value2);
    }
}

static void
plot (Cairo::RefPtr<Cairo::Context> cr, vector<SpectData>& spect, int n, int width, int height)
{
  for (size_t i = 0; i < spect.size(); i++)
    {
      double x = spect[i].freq / 8000 * width;
      double db_mag = bse_db_from_factor (spect[i].mag, -200);
      double xmag = (db_mag + 60) / 60;

      xmag = max (xmag, 0.05);
      // xmag is in range (1, 0.05)
      if (n == 0)
        {
          cr->move_to (x, 0.45 * height);
          cr->line_to (x, 0.45 * height - xmag * 0.5 * height);
        }
      else
        {
          cr->move_to (x, 0.55 * height);
          cr->line_to (x, 0.55 * height + xmag * 0.5 * height);
        }
    }
}

bool
MorphView::on_expose_event (GdkEventExpose* ev)
{
  const int width =  800 * hzoom;
  const int height = 600 * vzoom;

  size_t frame = 130;
  while (frame_data[frame].a.empty())
    {
      frame++;
    }
  printf ("frame=%zd\n", frame);

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


      FrameData& fd = frame_data[frame];
      cr->set_source_rgb (0.8, 0.0, 0.0);
      plot (cr, fd.a, 0, width, height);
      cr->stroke();

      cr->set_source_rgb (0.0, 0.8, 0.0);
      plot (cr, fd.b, 1, width, height);
      cr->stroke();

      cr->set_source_rgb (0.0, 0.0, 0.5);
      for (size_t i = 0; i < fd.lines.size(); i++)
        {
          double x1 = fd.lines[i].freq1 / 8000 * width;
          double x2 = fd.lines[i].freq2 / 8000 * width;
          cr->move_to (x1, 0.45 * height);
          cr->line_to (x2, 0.55 * height);
        }
      cr->stroke();
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
MorphView::force_redraw()
{
  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}

class MorphPlotWindow : public Gtk::Window
{
  Gtk::ScrolledWindow scrolled_win;
  MorphView           morph_view;
  Gtk::VBox           vbox;
  ZoomController      zoom_controller;

public:
  MorphPlotWindow (const string& filename)
  {
    morph_view.load (filename);

    set_border_width (10);
    set_default_size (800, 600);

    vbox.pack_start (scrolled_win);
    vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);

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
