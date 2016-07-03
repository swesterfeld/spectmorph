#!/usr/bin/python

# Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

import sys
import subprocess
import re

from PyQt5 import QtGui, QtCore, QtWidgets

command = ["smevalplayer"]
play_p = subprocess.Popen (command, stdin=subprocess.PIPE)

def play (wavfile):
  play_p.stdin.write ("play %s\n" % wavfile)

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

class Example(QtWidgets.QMainWindow):
  def __init__ (self):
    QtWidgets.QMainWindow.__init__(self)

    self.items = []
    config = parse_config ("x.cfg")
    if len (config) > 0:
      for x in config[0]:
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
        item.rating_label = rating_label
        grid_layout.addWidget (rating_label, 0, col)

        slider = QtWidgets.QSlider (QtCore.Qt.Vertical)
        slider.valueChanged.connect (lambda rating, item=item: self.on_rating_changed (item, rating))
        slider.setRange (0, 100)
        slider.setValue (100)
        slider.setTickInterval (20);
        slider.setTickPosition (QtWidgets.QSlider.TicksBothSides)
        grid_layout.addWidget (slider, 1, col)

      col += 1

    central_widget.setLayout (grid_layout)
    self.setCentralWidget (central_widget)

  def on_play (self, item):
    play (item.filename)

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
