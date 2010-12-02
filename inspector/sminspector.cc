/*
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtkmm.h>
#include <assert.h>
#include <bse/bseloader.h>
#include <bse/bsemathsignal.h>
#include <bse/bseblockutils.hh>
#include <sys/time.h>

#include <vector>
#include <string>

#include "smmain.hh"
#include "smfft.hh"
#include "smmath.hh"
#include "smmicroconf.hh"
#include "smwavset.hh"

using std::vector;
using std::string;
using std::max;

using namespace SpectMorph;

struct AnalysisParams
{
  double frame_size_ms;
  double frame_step_ms;
};

struct FFTResult
{
  vector<float> mags;
};

struct PixelArray
{
  size_t width;
  size_t height;
  vector<unsigned char> pixels;

public:
  PixelArray();

  bool empty();
  void resize (size_t width, size_t height);
  unsigned char *get_pixels();
  size_t get_rowstride();
  size_t get_height();
  size_t get_width();
};

PixelArray::PixelArray()
{
  width = 0;
  height = 0;
}

void
PixelArray::resize (size_t width, size_t height)
{
  this->width = width;
  this->height = height;

  pixels.clear();
  pixels.resize (width * height);
}

bool
PixelArray::empty()
{
  return (width == 0) && (height == 0);
}

unsigned char *
PixelArray::get_pixels()
{
  return &pixels[0];
}

size_t
PixelArray::get_rowstride()
{
  return width;
}

size_t
PixelArray::get_width()
{
  return width;
}

size_t
PixelArray::get_height()
{
  return height;
}

class TimeFreqView : public Gtk::DrawingArea
{
protected:
  vector<FFTResult> results;
  PixelArray  image;
  Glib::RefPtr<Gdk::Pixbuf> zimage;
  double hzoom, vzoom;

public:
  TimeFreqView (); //const string& filename);

  void load (const string& filename);
  bool on_expose_event (GdkEventExpose* ev);

  void set_hzoom (double new_hzoom);
  void set_vzoom (double new_vzoom);
};

TimeFreqView::TimeFreqView () //const string& filename)
{
  set_size_request (400, 400);
  hzoom = 1;
  vzoom = 1;

  //load (filename);
}

void
TimeFreqView::set_hzoom (double new_hzoom)
{
  hzoom = new_hzoom;
  zimage.clear();

  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}

void
TimeFreqView::set_vzoom (double new_vzoom)
{
  vzoom = new_vzoom;
  zimage.clear();

  Glib::RefPtr<Gdk::Window> win = get_window();
  if (win)
    {
      Gdk::Rectangle r (0, 0, get_allocation().get_width(), get_allocation().get_height());
      win->invalidate_rect (r, false);
    }
}

struct Options
{
  string program_name;
} options;

void
TimeFreqView::load (const string& filename)
{
  BseErrorType error;
  BseWaveFileInfo *wave_file_info = bse_wave_file_info_load (filename.c_str(), &error);
  if (!wave_file_info)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      exit (1);
    }

  BseWaveDsc *waveDsc = bse_wave_dsc_load (wave_file_info, 0, FALSE, &error);
  if (!waveDsc)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      exit (1);
    }

  GslDataHandle *dhandle = bse_wave_handle_create (waveDsc, 0, &error);
  if (!dhandle)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      exit (1);
    }

  error = gsl_data_handle_open (dhandle);
  if (error)
    {
      fprintf (stderr, "%s: can't open the input file %s: %s\n", options.program_name.c_str(), filename.c_str(), bse_error_blurb (error));
      exit (1);
    }

  if (gsl_data_handle_n_channels (dhandle) != 1)
    {
      fprintf (stderr, "Currently, only mono files are supported.\n");
      exit (1);
    }
  AnalysisParams analysis_params;
  analysis_params.frame_size_ms = 40;
  analysis_params.frame_step_ms = 10;

  size_t frame_size = analysis_params.frame_size_ms * gsl_data_handle_mix_freq (dhandle) / 1000.0;
  size_t block_size = 1;
  while (block_size < frame_size)
    block_size *= 2;

  vector<float> block (block_size);
  vector<float> window (block_size);

  size_t zeropad = 4;
  size_t fft_size = block_size * zeropad;

  float *fft_in = FFT::new_array_float (fft_size);
  float *fft_out = FFT::new_array_float (fft_size);

  for (guint i = 0; i < window.size(); i++)
    {
      if (i < frame_size)
        window[i] = bse_window_cos (2.0 * i / frame_size - 1.0);
      else
        window[i] = 0;
    }


  double len_ms = gsl_data_handle_length (dhandle) * 1000.0 / gsl_data_handle_mix_freq (dhandle); 
  for (double pos_ms = analysis_params.frame_step_ms * 0.5 - analysis_params.frame_size_ms; pos_ms < len_ms; pos_ms += analysis_params.frame_step_ms)
    {
      int64 pos = pos_ms / 1000.0 * gsl_data_handle_mix_freq (dhandle);
      int64 todo = block.size(), offset = 0;
      const int64 n_values = gsl_data_handle_length (dhandle);

      while (todo)
        {
          int64 r = 0;
          if ((pos + offset) < 0)
            {
              r = 1;
              block[offset] = 0;
            }
          else if (pos + offset < n_values)
            {
              r = gsl_data_handle_read (dhandle, pos + offset, todo, &block[offset]);
            }
          if (r > 0)
            {
              offset += r;
              todo -= r;
            }
          else  // last block
            {
              while (todo)
                {
                  block[offset++] = 0;
                  todo--;
                }
            }
        }
      Bse::Block::mul (block_size, &block[0], &window[0]);
      for (size_t i = 0; i < fft_size; i++)
        {
          if (i < block_size)
            fft_in[i] = block[i];
          else
            fft_in[i] = 0;
        }
      FFT::fftar_float (fft_size, fft_in, fft_out);
      FFTResult result;
      fft_out[1] = 0; // special packing
      for (size_t i = 0; i < fft_size; i += 2)      
        {
          double re = fft_out[i];
          double im = fft_out[i + 1];

          result.mags.push_back (sqrt (re * re + im * im));
        }
      results.push_back (result);
    }
}

float
value_scale (float value)
{
  if (true)
    {
      double db = bse_db_from_factor (value, -200);
      if (db > -90)
        return db + 90;
      else
        return 0;
    }
  else
    return value;
}

#define FRAC_SHIFT       12
#define FRAC_FACTOR      (1 << FRAC_SHIFT)
#define FRAC_HALF_FACTOR (1 << (FRAC_SHIFT - 1))

Glib::RefPtr<Gdk::Pixbuf>
zoom_rect (PixelArray& image, int destx, int desty, int destw, int desth, double hzoom, double vzoom)
{
  Glib::RefPtr<Gdk::Pixbuf> zimage = Gdk::Pixbuf::create (Gdk::COLORSPACE_RGB, false, 8, destw, desth);
  int hzoom_inv_frac = FRAC_FACTOR / hzoom;
  int vzoom_inv_frac = FRAC_FACTOR / vzoom;

  guchar *p = zimage->get_pixels();
  size_t  row_stride = zimage->get_rowstride();

  guchar *pixel_array_p          = image.get_pixels();
  size_t  pixel_array_row_stride = image.get_rowstride();

  for (int y = 0; y < desth; y++)
    {
      guchar *pp = (p + row_stride * y);
      size_t outy = ((y + desty) * vzoom_inv_frac + FRAC_HALF_FACTOR) >> FRAC_SHIFT;

      for (int x = 0; x < destw; x++)
        {
          size_t outx = ((x + destx) * hzoom_inv_frac + FRAC_HALF_FACTOR) >> FRAC_SHIFT;
          int color;
          if (outx < image.get_width() && outy < image.get_height())
            color = pixel_array_p[pixel_array_row_stride * outy + outx];
          else
            color = 0;
          pp[0] = color;
          pp[1] = color;
          pp[2] = color;
          pp += 3;
        }
    }
  return zimage;
}

bool
TimeFreqView::on_expose_event (GdkEventExpose *ev)
{
  if (image.empty())
    {
      size_t height = 0;
      if (!results.empty())
        height = results[0].mags.size();

      image.resize (results.size(), height);

      float max_value = 0;
      for (vector<FFTResult>::const_iterator fi = results.begin(); fi != results.end(); fi++)
        {
          for (vector<float>::const_iterator mi = fi->mags.begin(); mi != fi->mags.end(); mi++)
            {
              max_value = max (max_value, value_scale (*mi));
            }
        }
      guchar *p = image.get_pixels();
      size_t  row_stride = image.get_rowstride();
      for (size_t frame = 0; frame < results.size(); frame++)
        {
          for (size_t m = 0; m < results[frame].mags.size(); m++)
            {
              double value = value_scale (results[frame].mags[m]);
              value /= max_value;
              int y = results[frame].mags.size() - 1 - m;
              p[row_stride * y] = value * 255;
            }
          p++;
        }
    }
  /*
  if (!zimage)
    {
      zimage = image->scale_simple (image->get_width() * hzoom, image->get_height() * vzoom, Gdk::INTERP_BILINEAR);
      set_size_request (zimage->get_width(), zimage->get_height());
    }
  zimage->render_to_drawable (get_window(), get_style()->get_black_gc(), 0, 0, 0, 0, zimage->get_width(), zimage->get_height(),
                              Gdk::RGB_DITHER_NONE, 0, 0);
  */
  set_size_request (image.get_width() * hzoom, image.get_height() * vzoom);
  zimage = zoom_rect (image, ev->area.x, ev->area.y, ev->area.width, ev->area.height, hzoom, vzoom);
  zimage->render_to_drawable (get_window(), get_style()->get_black_gc(), 0, 0, ev->area.x, ev->area.y,
                              zimage->get_width(), zimage->get_height(),
                              Gdk::RGB_DITHER_NONE, 0, 0);
  return true;
}

class Index : public Gtk::Window
{
  string            smset_dir;
  Gtk::ComboBoxText smset_combobox;
  Gtk::VBox         index_vbox;

  struct ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
    ModelColumns()
    {
      add (m_col_name);
    }
    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
  };

  ModelColumns audio_chooser_cols;
  Glib::RefPtr<Gtk::ListStore> ref_tree_model;
  Gtk::TreeView tree_view;

public:
  Index (const string& filename);

  void on_combo_changed();
};

Index::Index (const string& filename)
{
  printf ("loading index: %s\n", filename.c_str());
  MicroConf cfg (filename);

  while (cfg.next())
    {
      string str;

      if (cfg.command ("smset", str))
        {
          smset_combobox.append_text (str);
        }
      else if (cfg.command ("smset_dir", str))
        {
          smset_dir = str;
        }
      else
        {
          cfg.die_if_unknown();
        }
    }

  set_border_width (10);
  set_default_size (200, 600);
  index_vbox.pack_start (smset_combobox, Gtk::PACK_SHRINK);
  ref_tree_model = Gtk::ListStore::create (audio_chooser_cols);
  tree_view.set_model (ref_tree_model);
  index_vbox.pack_start (tree_view);

  Gtk::TreeModel::Row row = *(ref_tree_model->append());
  row[audio_chooser_cols.m_col_name] = "hello";
  tree_view.append_column ("Name", audio_chooser_cols.m_col_name);

  add (index_vbox);
  show();
  show_all_children();
  smset_combobox.signal_changed().connect (sigc::mem_fun(*this, &Index::on_combo_changed));
}

void
Index::on_combo_changed()
{
  std::string file = smset_dir + "/" + smset_combobox.get_active_text().c_str();
  printf ("loading %s...\n", file.c_str());
  WavSet wset;
  BseErrorType error = wset.load (file);
  if (error)
    {
      fprintf (stderr, "sminspector: can't open input file: %s: %s\n", file.c_str(), bse_error_blurb (error));
      exit (1);
    }
  printf ("done.\n");
}

class MainWindow : public Gtk::Window
{
  Gtk::ScrolledWindow scrolled_win;
  TimeFreqView        time_freq_view;
  Gtk::Adjustment     hzoom_adjustment;
  Gtk::HScale         hzoom_scale;
  Gtk::Label          hzoom_label;
  Gtk::HBox           hzoom_hbox;
  Gtk::Adjustment     vzoom_adjustment;
  Gtk::HScale         vzoom_scale;
  Gtk::Label          vzoom_label;
  Gtk::HBox           vzoom_hbox;
  Gtk::VBox           vbox;
  Index               index;

public:
  MainWindow (const string& filename);

  void hzoom_changed();
  void vzoom_changed();
};

MainWindow::MainWindow (const string& filename) :
  //time_freq_view (filename),
  hzoom_adjustment (0.0, -1.0, 1.0, 0.01, 1.0, 0.0),
  hzoom_scale (hzoom_adjustment),
  vzoom_adjustment (0.0, -1.0, 1.0, 0.01, 1.0, 0.0),
  vzoom_scale (vzoom_adjustment),
  index (filename)
{
  set_border_width (10);
  set_default_size (800, 600);
  vbox.pack_start (scrolled_win);
  vbox.pack_start (hzoom_hbox, Gtk::PACK_SHRINK);
  hzoom_hbox.pack_start (hzoom_scale);
  hzoom_hbox.pack_start (hzoom_label, Gtk::PACK_SHRINK);
  hzoom_scale.set_draw_value (false);
  hzoom_label.set_text ("100.00%");
  hzoom_hbox.set_border_width (10);
  hzoom_scale.signal_value_changed().connect (sigc::mem_fun (*this, &MainWindow::hzoom_changed));
  vbox.pack_start (vzoom_hbox, Gtk::PACK_SHRINK);
  vzoom_hbox.pack_start (vzoom_scale);
  vzoom_hbox.pack_start (vzoom_label, Gtk::PACK_SHRINK);
  vzoom_scale.set_draw_value (false);
  vzoom_label.set_text ("100.00%");
  vzoom_hbox.set_border_width (10);
  vzoom_scale.signal_value_changed().connect (sigc::mem_fun (*this, &MainWindow::vzoom_changed));
  add (vbox);
  scrolled_win.add (time_freq_view);
  show_all_children();
}

void
MainWindow::hzoom_changed()
{
  double hzoom = pow (10, hzoom_adjustment.get_value());
  char buffer[1024];
  sprintf (buffer, "%3.2f%%", 100.0 * hzoom);
  hzoom_label.set_text (buffer);
  time_freq_view.set_hzoom (hzoom);
}

void
MainWindow::vzoom_changed()
{
  double vzoom = pow (10, vzoom_adjustment.get_value());
  char buffer[1024];
  sprintf (buffer, "%3.2f%%", 100.0 * vzoom);
  vzoom_label.set_text (buffer);
  time_freq_view.set_vzoom (vzoom);
}

static double
gettime()
{
  timeval tv;
  gettimeofday (&tv, 0);

  return tv.tv_sec + tv.tv_usec / 1000000.0;
}

int
main (int argc, char **argv)
{
  options.program_name = "sminspector";

  sm_init (&argc, &argv);

  Gtk::Main kit (argc, argv);

  assert (argc == 2 || argc == 3);
  if (argc == 3)
    {
      if (string (argv[2]) == "perf")
        {
          const double clocks_per_sec = 2500.0 * 1000 * 1000;
          const unsigned int runs = 10;

          PixelArray image;
          Glib::RefPtr<Gdk::Pixbuf> zimage;
          double hzoom = 1.3, vzoom = 1.5;
          image.resize (1024, 1024);

          // warmup run:
          zimage = zoom_rect (image, 50, 50, 300, 300, hzoom, vzoom);

          // timed runs:
          double start = gettime();
          for (unsigned int i = 0; i < runs; i++)
            zimage = zoom_rect (image, 50, 50, 300, 300, hzoom, vzoom);
          double end = gettime();

          printf ("zoom_rect: %f clocks/pixel\n", clocks_per_sec * (end - start) / (300 * 300) / runs);
          printf ("zoom_rect: %f Mpixel/s\n", 1.0 / ((end - start) / (300 * 300) / runs) / 1000 / 1000);

          return 0;
        }
      else
        {
          assert (false);
        }
    }

  MainWindow window (argv[1]);

  Gtk::Main::run (window);
}
