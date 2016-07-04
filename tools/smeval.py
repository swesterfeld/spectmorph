#!/usr/bin/python
# -*- coding: utf-8 -*-
# Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

# coding=utf-8

import sys
import subprocess
import re
import argparse
import random

from PyQt5 import QtGui, QtCore, QtWidgets

command = ["smevalplayer"]
play_p = subprocess.Popen (command, stdin=subprocess.PIPE)

def play (wavfile):
  play_p.stdin.write ("play %s\n" % wavfile)

def stop():
  play_p.stdin.write ("stop\n")

def debug_play():
  play_p.stdin.write ("debug\n")

# vvv----------------- copypasted from sminstbuilder -------------------------
def tokenize (input_str):
  class TState:
    BLANK = 1
    STRING = 2
    COMMENT = 3
    QUOTED_STRING = 4
    QUOTED_STRING_ESCAPED = 5

  def string_chars (ch):
    if ((ch >= 'a' and ch <= 'z') or
        (ch >= 'A' and ch <= 'Z') or
        (ch >= '0' and ch <= '9') or
         ch in ".:=/-_%{}"):
      return True
    return False

  def white_space (ch):
    return (ch == ' ' or ch == '\n' or ch == '\t' or  ch == '\r');

  input_str += "\n"
  state = TState.BLANK

  s = ""
  tokens = []

  for ch in input_str:
    if state == TState.BLANK and string_chars (ch):
      s += ch
      state = TState.STRING
    elif state == TState.BLANK and ch == '"':
      state = TState.QUOTED_STRING
    elif state == TState.BLANK and white_space (ch):
      pass # ignore more whitespaces if we've already seen one
    elif state == TState.STRING and string_chars (ch):
      s += ch
    elif ((state == TState.STRING and white_space (ch)) or
          (state == TState.QUOTED_STRING and ch == '"')):
      tokens += [ s ]
      s = "";
      state = TState.BLANK
    elif state == TState.QUOTED_STRING and ch == '\\':
      state = TState.QUOTED_STRING_ESCAPED
    elif state == TState.QUOTED_STRING:
      s += ch
    elif state == TState.QUOTED_STRING_ESCAPED:
      s += ch;
      state = TState.QUOTED_STRING
    elif ch == '#':
      state = TState.COMMENT
    elif state == TState.COMMENT:
      pass # ignore comments
    else:
      raise Exception ("Tokenizer error in char '" + ch + "'")
  if state != TState.BLANK and state != TState.COMMENT:
    raise Exception ("Parse Error in String: \"" + input_str + "\"")
  return tokens
# ^^^----------------- copypasted from sminstbuilder -------------------------

def parse_config (filename):
  f = open (filename, "r")

  in_block = False
  block = []
  config = []

  for line in f:
    line = re.sub ("#.*$", "", line)
    cmd = tokenize (line)
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

class Test:
  pass

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

class Example (QtWidgets.QMainWindow):
  def __init__ (self, args):
    QtWidgets.QMainWindow.__init__(self)

    self.cmdline_args = args

    config = parse_config (self.cmdline_args.config)
    self.tests = []
    for test_config in config:
      test = Test()
      reference_items = []
      rate_items = []
      test.title = ""
      for x in test_config:
        if len (x) == 2 and x[0] == "reference":
          item = TestItem()
          item.filename = x[1]
          item.reference = True
          reference_items.append (item)
        elif len (x) == 2 and x[0] == "rate":
          item = TestItem()
          item.filename = x[1]
          item.reference = False
          item.rating = 100
          rate_items.append (item)
        elif len (x) == 2 and x[0] == "title":
          test.title = x[1]

      # double blind test
      random.shuffle (rate_items)
      test.items = reference_items + rate_items
      self.tests.append (test)

    self.resize (1024, 768)
    self.test_number = 0
    self.reinit_ui (self.test_number)

    if (self.cmdline_args.debug):
      debug_play()

  def reinit_ui (self, n):
    if len (self.tests) == n:
      reply = QtWidgets.QMessageBox.question (self, 'Message', "Save Test Results and Quit?", QtWidgets.QMessageBox.Yes | QtWidgets.QMessageBox.No, QtWidgets.QMessageBox.No)
      if reply == QtWidgets.QMessageBox.Yes:
        with open (self.cmdline_args.results, "w") as result_file:
          for test in self.tests:
            for item in test.items:
              if not item.reference:
                result_file.write ("%s %d\n" % (item.filename, item.rating))
        self.close()
      else:
        # back to last test
        n -= 1
        self.test_number -= 1

    if len (self.tests) > n:
      self.items = self.tests[self.test_number].items
      self.title = self.tests[self.test_number].title
      self.initUI()
    else:
      self.items = []
      self.title = ""

  def initUI (self):
    central_widget = QtWidgets.QWidget (self)
    grid_layout = QtWidgets.QGridLayout()

    col = 0
    rate_count = 0
    for item in self.items:
      if (self.cmdline_args.debug):
        btn_text = "Play - " + item.filename
      else:
        if item.reference:
          btn_text = "Reference %d" % (col + 1)
        else:
          btn_text = "%c" % (rate_count + 65)
          rate_count += 1

      button = QtWidgets.QPushButton (btn_text, self)
      button.clicked.connect (lambda checked, item=item: self.on_play (item))
      grid_layout.addWidget (button, 4, col)
      item.play_button = button

      if not item.reference:
        rating_label = QtWidgets.QLabel()
        rating_label.setFont (QtGui.QFont("Times", 16, QtGui.QFont.Bold))
        item.rating_label = rating_label
        item.rating_label.setAlignment (QtCore.Qt.AlignCenter)
        grid_layout.addWidget (rating_label, 2, col)

        slider = QtWidgets.QSlider (QtCore.Qt.Vertical)
        slider.valueChanged.connect (lambda rating, item=item: self.on_rating_changed (item, rating))
        slider.setRange (0, 100)
        slider.setValue (item.rating)
        slider.setTickInterval (20);
        slider.setTickPosition (QtWidgets.QSlider.TicksBothSides)
        slider.setEnabled (False)
        item.slider = slider
        grid_layout.addWidget (slider, 3, col, 1, 1, QtCore.Qt.AlignHCenter)

      col += 1

    next_button = QtWidgets.QPushButton ("Next >>")
    next_button.clicked.connect (self.on_next_clicked)
    grid_layout.addWidget (next_button, 0, col - 1)

    prev_button = QtWidgets.QPushButton ("<< Prev")
    prev_button.clicked.connect (self.on_prev_clicked)
    grid_layout.addWidget (prev_button, 0, col - 2)

    test_label = QtWidgets.QLabel()
    test_label.setFont (QtGui.QFont("Times", 32, QtGui.QFont.Bold))
    test_label.setText ("Test %d/%d: %s" % (self.test_number + 1, len (self.tests), self.title))
    grid_layout.addWidget (test_label, 1, 0, 1, col, QtCore.Qt.AlignHCenter)

    central_widget.setLayout (grid_layout)
    self.setCentralWidget (central_widget)

    reference_scale = ReferenceScale ()
    grid_layout.addWidget (reference_scale, 3, 0)

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

  def on_prev_clicked (self):
    if (self.test_number > 0):
      self.test_number -= 1
    self.reinit_ui (self.test_number)
    stop()

  def on_rating_changed (self, item, rating):
    item.rating_label.setText ("%d" % rating)
    item.rating = rating

def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--debug', action='store_true')
  parser.add_argument('config', action='store')
  parser.add_argument('results', action='store')
  parsed_args, unparsed_args = parser.parse_known_args()

  app = QtWidgets.QApplication (sys.argv[:1] + unparsed_args)
  ex = Example (parsed_args)
  ex.show()
  sys.exit(app.exec_())

if __name__ == "__main__":
  main()
