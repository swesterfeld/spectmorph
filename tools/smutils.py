import sys, subprocess

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

def die (message):
  print >> sys.stderr, "%s: %s" % (sys.argv[0], message)
  exit (1)

def system_or_die (command):
  print "+++ %s" % command
  sys.stdout.flush()
  return_code = subprocess.call (command, shell=True)
  if return_code != 0:
    die ("executing command '%s' failed, return_code=%d" % (command, return_code))
