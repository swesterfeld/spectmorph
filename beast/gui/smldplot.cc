#include <gtkmm.h>
#include <assert.h>
#include <sys/time.h>
#include <bse/bse.h>
#include <bse/bsemathsignal.h>
#include "smmain.hh"
#include "smzoomcontroller.hh"
#include "smmath.hh"
#include <map>

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

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
  vector<FrameData>                     frame_data;
  std::map<string, vector<FrameData> >  frame_data_cache;

  double hzoom;
  double vzoom;
  int    view_width;
  int    view_height;
  size_t frame;
public:
  MorphView();

  sigc::signal<void, string, string> signal_mouse_changed;

  bool on_expose_event (GdkEventExpose* ev);
  void load (const string& filename);
  void set_zoom (double hzoom, double vzoom);
  void set_frame (size_t frame);
  void force_redraw();
  size_t frames();
  bool on_motion_notify_event (GdkEventMotion *event);
};

MorphView::MorphView()
{
  hzoom = 1;
  vzoom = 1;
  frame = 0;

  add_events (Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_MOTION_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
}

void
MorphView::load (const string& filename)
{
  vector<FrameData>& cached_fd = frame_data_cache[filename];
  if (!cached_fd.empty())
    {
      frame_data = cached_fd;
      return;
    }
  frame_data.clear();
  printf ("loading %s...\n", filename.c_str());

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
  cached_fd = frame_data;
}

bool
MorphView::on_expose_event (GdkEventExpose* ev)
{
  view_width =  800 * hzoom;
  view_height = 600 * vzoom;

  const double start_t = gettime();

  set_size_request (view_width, view_height);

  Glib::RefPtr<Gdk::Window> window = get_window();
  if (window)
    {
      Cairo::RefPtr<Cairo::ImageSurface> image_surface =
        Cairo::ImageSurface::create (Cairo::FORMAT_RGB24, ev->area.width, ev->area.height);

      Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create (image_surface);

      cr->save();
      cr->set_source_rgb (1.0, 1.0, 1.0);   // white
      cr->paint();
      cr->restore();

      cr->set_line_width (2.0);

      // clip to the area indicated by the expose event so that we only redraw
      // the portion of the window that needs to be redrawn
      cr->rectangle (0, 0, ev->area.width, ev->area.height);
      cr->clip();

      // translate coords, so we draw operations will draw absolute coordinates
      // relative to expose event x/y
      cr->translate (-ev->area.x, -ev->area.y);

      for (size_t i = 0; i < frame_data.size(); i++)
        {
          cr->set_line_width (1.0);
          for (size_t j = 0; j < frame_data[i].lines.size(); j++)
            {
              const LineData& line_data = frame_data[i].lines[j];

              double db_mag = bse_db_from_factor (line_data.mag, -200);
              double color = 1.0 - CLAMP ((100 + db_mag) / 100, 0.0, 1.0);
              cr->set_source_rgb (color, color, color);

              double y1 = (1 - line_data.freq1 / 22050) * view_height;
              double y2 = (1 - line_data.freq2 / 22050) * view_height;
              double x1 = double (i - 1.0) / frame_data.size() * view_width;
              double x2 = double (i) / frame_data.size() * view_width;
              cr->move_to (x1, y1);
              cr->line_to (x2, y2);
              cr->stroke();
            }
          cr->set_line_width (1.0);
          for (size_t j = 0; j < frame_data[i].freqs.size(); j++)
            {
              const FreqData& freq_data = frame_data[i].freqs[j];

              double db_mag = bse_db_from_factor (freq_data.mag, -200);
              double color = 1.0 - CLAMP ((100 + db_mag) / 100, 0.0, 1.0);
              cr->set_source_rgb (1, color, color);

              double y = (1 - freq_data.freq / 22050) * view_height;
              double x = double (i) / frame_data.size() * view_width;
              cr->move_to (x, y - 5);
              cr->line_to (x, y + 5);
              cr->stroke();
            }
        }

      Cairo::RefPtr<Cairo::Context> win_ctx = window->create_cairo_context();

      win_ctx->set_source (image_surface, ev->area.x, ev->area.y);
      win_ctx->save();
      win_ctx->rectangle (ev->area.x, ev->area.y, ev->area.width, ev->area.height);
      win_ctx->clip();
      win_ctx->paint();
      win_ctx->restore();
    }
  const double end_t = gettime();
  (void) (end_t - start_t);
  //printf ("drawing time: %.2f\n", end_t - start_t);

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

bool
MorphView::on_motion_notify_event (GdkEventMotion *event)
{
  double height = view_height;
  double width =  view_width;
  double pos = (event->x / width) * frame_data.size();
  int time = 0;
  for (size_t i = 0; i < frame_data.size(); i++)
    {
      if (i > pos)
        break;
      else
        time = 1000 * frame_data[i].index / 44100.0;
    }
  int ms = time % 1000;
  time /= 1000;
  int s = time % 60;
  time /= 60;
  int m = time;

  double freq = (1 - event->y / height) * 22050;

  signal_mouse_changed (Birnet::string_printf ("Time: %02d:%02d:%03d ms", m, s, ms),
                        Birnet::string_printf ("Freq: %.2f Hz", freq));

  return true;
}

class MorphPlotWindow : public Gtk::Window
{
  Gtk::ScrolledWindow scrolled_win;
  MorphView           morph_view;
  Gtk::VBox           vbox;
  Gtk::HBox           hbox;
  Gtk::Label          time_label;
  Gtk::Label          freq_label;
  Gtk::Label          filename_label;
  ZoomController      zoom_controller;

  Gtk::ComboBoxText   filename_combobox;

public:
  MorphPlotWindow (int argc, char **argv) :
    zoom_controller (5000, 5000)
  {
    set_border_width (10);
    set_default_size (800, 600);

    vbox.pack_start (scrolled_win);
    vbox.pack_start (zoom_controller, Gtk::PACK_SHRINK);

    filename_label.set_text ("Filename:");

    hbox.pack_start (time_label);
    hbox.pack_start (freq_label);
    hbox.pack_start (filename_label);
    hbox.pack_start (filename_combobox);
    vbox.pack_start (hbox, Gtk::PACK_SHRINK);

    for (int i = 1; i < argc; i++)
      filename_combobox.append_text (argv[i]);

    filename_combobox.signal_changed().connect (sigc::mem_fun (*this, &MorphPlotWindow::on_filename_changed));

    scrolled_win.add (morph_view);
    add (vbox);

    zoom_controller.signal_zoom_changed.connect (sigc::mem_fun (*this, &MorphPlotWindow::on_zoom_changed));
    morph_view.signal_mouse_changed.connect (sigc::mem_fun (*this, &MorphPlotWindow::on_mouse_changed));

    show_all_children();
  }

  void
  on_mouse_changed (const string& time, const string& freq)
  {
    time_label.set_text (time);
    freq_label.set_text (freq);
  }

  void
  on_zoom_changed()
  {
    morph_view.set_zoom (zoom_controller.get_hzoom(), zoom_controller.get_vzoom());
  }

  void
  on_filename_changed()
  {
    set_title ("smldplot - " + filename_combobox.get_active_text());
    morph_view.load (filename_combobox.get_active_text());
    morph_view.force_redraw();
  }
};

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

  Gtk::Main kit (argc, argv);

  if (argc < 2)
    {
      printf ("usage: %s <filename> ...\n", argv[0]);
      exit (1);
    }

  MorphPlotWindow window (argc, argv);
  Gtk::Main::run (window);
}
