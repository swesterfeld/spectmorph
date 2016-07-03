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

class Example(QtWidgets.QMainWindow):

    def __init__(self):
        QtWidgets.QMainWindow.__init__(self)

        self.initUI()
        self.wavs = []
        config = parse_config ("x.cfg")
        if len (config) > 0:
          for x in config[0]:
            self.wavs.append (x[1])

    def initUI(self):
        self.setGeometry(300,300,200,200)

        self.b1 = QtWidgets.QPushButton("Play", self)
        self.b1.clicked.connect(self.Play)
        self.b1.move(50, 80)

        self.b2 = QtWidgets.QPushButton("Play2", self)
        self.b2.clicked.connect(self.Play2)
        self.b2.move(50, 120)

        self.b2 = QtWidgets.QPushButton("Play3", self)
        self.b2.clicked.connect(self.Play3)
        self.b2.move(50, 160)

    def Play(self):
        play (self.wavs[0])

    def Play2(self):
        play (self.wavs[1])

    def Play3(self):
        play (self.wavs[2])

def main():
    app = QtWidgets.QApplication(sys.argv)
    ex = Example()
    ex.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
