// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_PLAYER_WINDOW_HH
#define SPECTMORPH_PLAYER_WINDOW_HH

#include <jack/jack.h>

#include <QWidget>
#include <QSlider>
#include <QLabel>

#include "smsimplejackplayer.hh"

namespace SpectMorph {

class Navigator;
class PlayerWindow : public QWidget
{
  Q_OBJECT

  Navigator          *navigator;
  QSlider            *volume_slider;
  QLabel             *volume_label;

  SimpleJackPlayer    jack_player;

public:
  PlayerWindow (Navigator *navigator);

public slots:
  void on_play_clicked();
  void on_stop_clicked();
  void on_next_clicked();
  void on_prev_clicked();
  void on_volume_changed (int new_volume);

signals:
  void next_sample();
  void prev_sample();
};

}

#endif

