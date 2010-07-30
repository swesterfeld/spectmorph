/* 
 * Copyright (C) 2009-2010 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPECTMORPH_INFILE_HH
#define SPECTMORPH_INFILE_HH

#include <stdio.h>
#include <string>
#include <vector>
#include <set>

#include "smstdioin.hh"
#include "smmmapin.hh"

namespace SpectMorph
{

class InFile
{
public:
  enum Event {
    NONE,
    END_OF_FILE,
    BEGIN_SECTION,
    END_SECTION,
    INT,
    STRING,
    FLOAT,
    FLOAT_BLOCK,
    BLOB
  };

protected:
  GenericIn         *file;
  Event              current_event;
  std::string        current_event_str;
  int                current_event_int;
  std::string        current_event_data;
  float              current_event_float;
  std::vector<float> current_event_float_block;
  size_t             current_event_blob_pos;
  size_t             current_event_blob_size;

  std::set<std::string> skip_events;

  void        read_raw_string (std::string& str);
  int         read_raw_int();
  float       read_raw_float();
  void        read_raw_float_block (std::vector<float>& fb);
  void        skip_raw_float_block();

public:
  InFile (const std::string& filename)
  {
    file = MMapIn::open (filename);
    current_event = NONE;
  }
  bool
  open_ok()
  {
    return file != NULL;
  }
  Event        event();
  std::string  event_name();
  float        event_float();
  int          event_int();
  std::string  event_data();
  const std::vector<float>& event_float_block();
  void         next_event();
  void         add_skip_event (const std::string& event);

  GenericIn   *open_blob();
};

}

#endif /* SPECTMORPH_INFILE_HH */
