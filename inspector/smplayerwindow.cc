// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smsamplewindow.hh"
#include "smnavigator.hh"
#include "smmemout.hh"
#include "smmmapin.hh"
#include "smlivedecoder.hh"
#include "smutils.hh"

#include <QPushButton>
#include <QHBoxLayout>
#include <QShortcut>
#include <QKeySequence>

#include <iostream>
#include <jack/jack.h>

using namespace SpectMorph;

using std::vector;

PlayerWindow::PlayerWindow (Navigator *navigator) :
  navigator (navigator),
  jack_player ("sminspector")
{
  setWindowTitle ("Player");
  resize (300, 10); /* h = 10 is too small, but the window will use the minimum height instead */

  QPushButton *prev_button = new QPushButton ("<< Prev (-)");
  QPushButton *next_button = new QPushButton ("Next (+) >>");
  QPushButton *play_button = new QPushButton ("Play (P) ");
  QPushButton *stop_button = new QPushButton ("Stop");

  QShortcut *next_shortcut = new QShortcut (QKeySequence (Qt::Key_Plus), this);
  QShortcut *prev_shortcut = new QShortcut (QKeySequence (Qt::Key_Minus), this);
  QShortcut *play_shortcut = new QShortcut (QKeySequence (Qt::Key_P), this);

  connect (prev_button, SIGNAL (clicked()), this, SLOT (on_prev_clicked()));
  connect (next_button, SIGNAL (clicked()), this, SLOT (on_next_clicked()));
  connect (play_button, SIGNAL (clicked()), this, SLOT (on_play_clicked()));
  connect (stop_button, SIGNAL (clicked()), this, SLOT (on_stop_clicked()));

  connect (prev_shortcut, SIGNAL (activated()), this, SLOT (on_prev_clicked()));
  connect (next_shortcut, SIGNAL (activated()), this, SLOT (on_next_clicked()));
  connect (play_shortcut, SIGNAL (activated()), this, SLOT (on_play_clicked()));

  QHBoxLayout *button_hbox = new QHBoxLayout();
  QHBoxLayout *volume_hbox = new QHBoxLayout();
  QVBoxLayout *vbox = new QVBoxLayout();

  volume_label = new QLabel();
  volume_slider = new QSlider (Qt::Horizontal, this);
  volume_slider->setRange (-96000, 24000);
  connect (volume_slider, SIGNAL (valueChanged(int)), this, SLOT (on_volume_changed(int)));
  on_volume_changed (0);

  volume_hbox->addWidget (new QLabel ("Volume"));
  volume_hbox->addWidget (volume_slider);
  volume_hbox->addWidget (volume_label);

  button_hbox->addWidget (prev_button);
  button_hbox->addWidget (play_button);
  button_hbox->addWidget (stop_button);
  button_hbox->addWidget (next_button);

  vbox->addLayout (button_hbox);
  vbox->addLayout (volume_hbox);
  setLayout (vbox);
}

void
PlayerWindow::on_play_clicked()
{
  Audio *audio = navigator->get_audio();

  jack_player.play (audio, !navigator->spectmorph_signal_active());
}

void
PlayerWindow::on_stop_clicked()
{
  jack_player.stop();
}

void
PlayerWindow::on_next_clicked()
{
  Q_EMIT next_sample();
}

void
PlayerWindow::on_prev_clicked()
{
  Q_EMIT prev_sample();
}

void
PlayerWindow::on_volume_changed (int new_volume_int)
{
  double new_volume = new_volume_int / 1000.0;
  double new_decoder_volume = bse_db_to_factor (new_volume);
  volume_label->setText (string_locale_printf ("%.1f dB", new_volume).c_str());

  jack_player.set_volume (new_decoder_volume);
}
