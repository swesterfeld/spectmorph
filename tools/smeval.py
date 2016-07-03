#!/usr/bin/python
# -*- coding: utf-8 -*-
# Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

# coding=utf-8

import sys
import subprocess
import re

from PyQt5 import QtGui, QtCore, QtWidgets

command = ["smevalplayer"]
play_p = subprocess.Popen (command, stdin=subprocess.PIPE)

def play (wavfile):
  play_p.stdin.write ("play %s\n" % wavfile)

def stop():
  play_p.stdin.write ("stop\n")

def parse_config (filename):
  f = open (filename, "r")

  in_block = False
  block = []
  config = []

  for line in f:
    line = re.sub ("#.*$", "", line)
    cmd = line.split()
    if not in_block:
      if cmd == ["{"]:
        in_block = 1
    else: # in_block
      if cmd == ["}"]:
        config.append (block)
        in_block = False
        block = []
      elif cmd == []: # empty line or comment
        pass
      else:
        block.append (cmd)

  return config

class TestItem:
  pass

class ReferenceScale(QtWidgets.QWidget):
  def __init__(self):
    super(ReferenceScale, self).__init__()

  def paintEvent(self, event):
    qp = QtGui.QPainter()
    qp.begin(self)
    #texts = [ "Excellent", "Good", "Fair", "Poor", "Bad" ]
    texts = [ "ausgezeichnet", "gut", "ordentlich", "mäßig", "mangelhaft" ]

    for i in range (len (texts)):
      y0 = self.height() / len (texts) * i
      y1 = self.height() / len (texts) * (i + 1)
      qp.setPen(QtGui.QColor(168, 34, 3))
      qp.setFont(QtGui.QFont('Decorative', 16))
      qp.drawText(0, y0, self.width(), y1 - y0, QtCore.Qt.AlignCenter, texts[i])

    DELTA = 10
    TEXT_WIDTH = 30
    for i in range (len (texts) + 1):
      y = (self.height() - 2 * DELTA) / (len (texts)) * i + DELTA
      qp.drawLine(0, y, self.width() / 2 - TEXT_WIDTH, y)
      qp.drawLine(self.width() / 2 + TEXT_WIDTH, y, self.width(), y)
      qp.drawText (0, y-DELTA, self.width(), 2 * DELTA, QtCore.Qt.AlignCenter, "%d" % (100 - i * 20))

    qp.end()

class Example(QtWidgets.QMainWindow):
  def __init__ (self):
    QtWidgets.QMainWindow.__init__(self)

    self.resize (1024, 768)
    self.test_number = 0
    self.reinit_ui (self.test_number)

  def reinit_ui (self, n):
    self.items = []
    config = parse_config ("x.cfg")
    if len (config) > n:
      for x in config[n]:
        if len (x) == 2 and x[0] == "reference":
          item = TestItem()
          item.filename = x[1]
          item.reference = True
          self.items.append (item)
        elif len (x) == 2 and x[0] == "rate":
          item = TestItem()
          item.filename = x[1]
          item.reference = False
          self.items.append (item)

    self.initUI()

  def initUI (self):
    central_widget = QtWidgets.QWidget (self)
    grid_layout = QtWidgets.QGridLayout()

    col = 0
    for item in self.items:
      button = QtWidgets.QPushButton ("Play - " + item.filename, self)
      button.clicked.connect (lambda checked, item=item: self.on_play (item))
      grid_layout.addWidget (button, 2, col)
      item.play_button = button

      if not item.reference:
        rating_label = QtWidgets.QLabel()
        rating_label.setFont (QtGui.QFont("Times", 16, QtGui.QFont.Bold))
        item.rating_label = rating_label
        item.rating_label.setAlignment (QtCore.Qt.AlignCenter)
        grid_layout.addWidget (rating_label, 0, col)

        slider = QtWidgets.QSlider (QtCore.Qt.Vertical)
        slider.valueChanged.connect (lambda rating, item=item: self.on_rating_changed (item, rating))
        slider.setRange (0, 100)
        slider.setValue (100)
        slider.setTickInterval (20);
        slider.setTickPosition (QtWidgets.QSlider.TicksBothSides)
        slider.setEnabled (False)
        item.slider = slider
        grid_layout.addWidget (slider, 1, col, 1, 1, QtCore.Qt.AlignHCenter)

      col += 1

    next_button = QtWidgets.QPushButton ("Next >>")
    next_button.clicked.connect (self.on_next_clicked)
    grid_layout.addWidget (next_button, 2, col)

    central_widget.setLayout (grid_layout)
    self.setCentralWidget (central_widget)

    reference_scale = ReferenceScale ()
    grid_layout.addWidget (reference_scale, 1, 0)

  def on_play (self, item):
    play (item.filename)

    # only allow rating the item that we're currently playing
    for other_item in self.items:
      if not other_item.reference:
        other_item.slider.setEnabled (item == other_item)

  def on_next_clicked (self):
    self.test_number += 1
    self.reinit_ui (self.test_number)
    stop()

  def on_rating_changed (self, item, rating):
    print item.filename, rating
    item.rating_label.setText ("%d" % rating)

def main():
  app = QtWidgets.QApplication(sys.argv)
  ex = Example()
  ex.show()
  sys.exit(app.exec_())

if __name__ == "__main__":
  main()
