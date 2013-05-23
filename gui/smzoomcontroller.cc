// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smzoomcontroller.hh"
#include "smutils.hh"
#include <math.h>
#include <stdio.h>

#include <glib.h>

#include <QSlider>
#include <QVBoxLayout>

using namespace SpectMorph;

using std::string;

ZoomController::ZoomController (QObject *parent, double hzoom_max, double vzoom_max) :
  QObject (parent)
{
  init();
  hzoom_slider->setRange (-1000, (log10 (hzoom_max) - 2) * 1000);
  vzoom_slider->setRange (-1000, (log10 (vzoom_max) - 2) * 1000);
}

ZoomController::ZoomController (QObject *parent, double hzoom_min, double hzoom_max, double vzoom_min, double vzoom_max) :
  QObject (parent)
{
  init();
  hzoom_slider->setRange ((log10 (hzoom_min) - 2) * 1000, (log10 (hzoom_max) - 2) * 1000);
  vzoom_slider->setRange ((log10 (vzoom_min) - 2) * 1000, (log10 (vzoom_max) - 2) * 1000);
}

void
ZoomController::init()
{
  vscrollbar = NULL;
  hscrollbar = NULL;

  hzoom_text = new QLabel ("HZoom");
  hzoom_slider = new QSlider (Qt::Horizontal);
  hzoom_label  = new QLabel();

  vzoom_text = new QLabel ("VZoom");
  vzoom_slider = new QSlider (Qt::Horizontal);
  vzoom_label  = new QLabel();

  connect (hzoom_slider, SIGNAL (valueChanged (int)), this, SLOT (on_hzoom_changed()));
  connect (vzoom_slider, SIGNAL (valueChanged (int)), this, SLOT (on_vzoom_changed()));

  old_hzoom = 1;
  old_vzoom = 1;

  on_hzoom_changed();
  on_vzoom_changed();
}

double
ZoomController::get_hzoom()
{
  return pow (10, hzoom_slider->value() / 1000.0);
}

double
ZoomController::get_vzoom()
{
  return pow (10, vzoom_slider->value() / 1000.0);
}

void
ZoomController::on_hzoom_changed()
{
  double hzoom = get_hzoom();
  string s = string_locale_printf ("%3.2f%%", 100.0 * hzoom);
  hzoom_label->setText (s.c_str());

  if (hscrollbar)
    {
      const double hfactor = hzoom / old_hzoom;
      hscrollbar->setValue (hfactor * hscrollbar->value() + ((hfactor - 1) * hscrollbar->pageStep() / 2));
    }
  old_hzoom = hzoom;
  Q_EMIT zoom_changed();
}

void
ZoomController::on_vzoom_changed()
{
  double vzoom = get_vzoom();
  string s = string_locale_printf ("%3.2f%%", 100.0 * vzoom);
  vzoom_label->setText (s.c_str());

  if (vscrollbar)
    {
      const double vfactor = vzoom / old_vzoom;
      vscrollbar->setValue (vfactor * vscrollbar->value() + ((vfactor - 1) * vscrollbar->pageStep() / 2));
    }
  old_vzoom = vzoom;
  Q_EMIT zoom_changed();
}

void
ZoomController::set_vscrollbar (QScrollBar *scrollbar)
{
  vscrollbar = scrollbar;
}

void
ZoomController::set_hscrollbar (QScrollBar *scrollbar)
{
  hscrollbar = scrollbar;
}

QWidget*
ZoomController::hwidget (int i)
{
  switch (i)
    {
      case 0: return hzoom_text;
      case 1: return hzoom_slider;
      case 2: return hzoom_label;
      default: g_assert_not_reached();
    }
}

QWidget*
ZoomController::vwidget (int i)
{
  switch (i)
    {
      case 0: return vzoom_text;
      case 1: return vzoom_slider;
      case 2: return vzoom_label;
      default: g_assert_not_reached();
    }
}
