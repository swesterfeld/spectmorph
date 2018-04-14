/* MicroConf - minimal configuration framework
 * Copyright (C) 2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include <stdlib.h>
#include <assert.h>
#include <glib.h>
#include "smmicroconf.hh"

using std::string;
using std::vector;
using namespace SpectMorph;

MicroConf::MicroConf (const string& filename)
{
  cfg_file = fopen (filename.c_str(), "r");

  current_file = filename;
  current_no = 0;
  m_number_format = I18N;
}

MicroConf::~MicroConf()
{
  if (cfg_file)
    {
      fclose (cfg_file);
      cfg_file = NULL;
    }
}

bool
MicroConf::open_ok()
{
  return cfg_file != NULL;
}

static bool
is_newline (char ch)
{
  return (ch == '\n' || ch == '\r');
}

bool
MicroConf::next()
{
  assert (cfg_file != NULL);

  char s[1024];

  if (!fgets (s, 1024, cfg_file))
    return false; // eof

  current_line = s;
  current_no++;

  // strip newline at end of input
  while (!current_line.empty() && is_newline (current_line[current_line.size() - 1]))
    current_line.resize (current_line.size() - 1);

  tokenizer_error = !tokenize();

  return true;
}

string
MicroConf::line()
{
  return current_line;
}

static bool
string_chars (char ch)
{
  if ((ch >= 'A' && ch <= 'Z')
  ||  (ch >= '0' && ch <= '9')
  ||  (ch >= 'a' && ch <= 'z')
  ||  (ch == '.')
  ||  (ch == ':')
  ||  (ch == '=')
  ||  (ch == '/')
  ||  (ch == '-')
  ||  (ch == '_'))
    return true;

  return false;
}

static bool
white_space (char ch)
{
  return (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r');
}

bool
MicroConf::tokenize()
{
  enum { BLANK, STRING, QUOTED_STRING, QUOTED_STRING_ESCAPED, COMMENT } state = BLANK;
  string s;

  string xline = current_line + '\n';
  tokens.clear();
  for (string::const_iterator i = xline.begin(); i != xline.end(); i++)
    {
      if (state == BLANK && string_chars (*i))
        {
          state = STRING;
          s += *i;
        }
      else if (state == BLANK && *i == '"')
        {
          state = QUOTED_STRING;
        }
      else if (state == BLANK && white_space (*i))
        {
          // ignore more whitespaces if we've already seen one
        }
      else if (state == STRING && string_chars (*i))
        {
          s += *i;
        }
      else if ((state == STRING && white_space (*i))
           ||  (state == QUOTED_STRING && *i == '"'))
        {
          tokens.push_back (s);
          s = "";
          state = BLANK;
        }
      else if (state == QUOTED_STRING && *i == '\\')
        {
          state = QUOTED_STRING_ESCAPED;
        }
      else if (state == QUOTED_STRING)
        {
          s += *i;
        }
      else if (state == QUOTED_STRING_ESCAPED)
        {
          s += *i;
          state = QUOTED_STRING;
        }
      else if (*i == '#')
        {
          state = COMMENT;
        }
      else if (state == COMMENT)
        {
          // ignore comments
        }
      else
        {
          return false;
        }
    }
  return state == BLANK;
}

bool
MicroConf::convert (const std::string& token, int& arg)
{
  arg = atoi (token.c_str());
  return true;
}

bool
MicroConf::convert (const std::string& token, double& arg)
{
  if (m_number_format == I18N)
    arg = atof (token.c_str());
  else
    arg = g_ascii_strtod (token.c_str(), NULL);
  return true;
}

bool
MicroConf::convert (const std::string& token, std::string& arg)
{
  arg = token;
  return true;
}

void
MicroConf::die_if_unknown()
{
  if (tokens.size())
    {
      fprintf (stderr, "configuration file %s: bad line number %d: %s\n",
               current_file.c_str(), current_no, current_line.c_str());
      exit (1);
    }
}

void
MicroConf::set_number_format (NumberFormat new_nf)
{
  m_number_format = new_nf;
}

MicroConf::NumberFormat
MicroConf::number_format()
{
  return m_number_format;
}
