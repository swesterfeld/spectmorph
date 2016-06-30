#!/usr/bin/python

# Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

import sys
import subprocess

from PyQt5 import QtGui, QtCore, QtWidgets

command = ["smevalplayer"]
play_p = subprocess.Popen (command, stdin=subprocess.PIPE)

def play (wavfile):
  play_p.stdin.write ("play %s\n" % wavfile)

class Example(QtWidgets.QMainWindow):

    def __init__(self):
        QtWidgets.QMainWindow.__init__(self)

        self.initUI()
        #self.wav1 = "/home/stefan/src/diplom/evaluation/grand-piano/clipped-note-24.wav"
        #self.wav2 = "/home/stefan/src/diplom/evaluation/grand-piano/clipped-note-31.wav"
        self.wav1 = "clipped-note-60.wav"
        self.wav2 = "clipped-note-60.sm.wav"

    def initUI(self):
        self.setGeometry(300,300,200,200)

        self.b1 = QtWidgets.QPushButton("Play", self)
        self.b1.clicked.connect(self.Play)
        self.b1.move(50, 80)

        self.b2 = QtWidgets.QPushButton("Play2", self)
        self.b2.clicked.connect(self.Play2)
        self.b2.move(50, 120)

    def Play(self):
        play (self.wav1)

    def Play2(self):
        play (self.wav2)




def main():
    app = QtWidgets.QApplication(sys.argv)
    ex = Example()
    ex.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
