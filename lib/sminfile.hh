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
    READ_ERROR,
    BEGIN_SECTION,
    END_SECTION,
    BOOL,
    INT,
    STRING,
    FLOAT,
    FLOAT_BLOCK,
    BLOB,
    BLOB_REF
  };

protected:
  GenericIn         *file;
  bool               file_delete;
  Event              current_event;
  std::string        current_event_str;
  bool               current_event_bool;
  int                current_event_int;
  std::string        current_event_data;
  float              current_event_float;
  std::vector<float> current_event_float_block;
  size_t             current_event_blob_pos;
  size_t             current_event_blob_size;
  std::string        current_event_blob_sum;
  std::string        m_file_type;
  int                m_file_version;

  std::set<std::string> skip_events;

  bool        read_raw_bool (bool& b);
  bool        read_raw_string (std::string& str);
  bool        read_raw_int (int &i);
  bool        read_raw_float (float &f);
  bool        read_raw_float_block (std::vector<float>& fb);
  bool        skip_raw_float_block();

  void        read_file_type_and_version();

public:
  InFile (const std::string& filename);
  InFile (GenericIn *file);
  ~InFile();

  bool
  open_ok()
  {
    return file != NULL;
  }
  Event        event();
  std::string  event_name();
  float        event_float();
  int          event_int();
  bool         event_bool();
  std::string  event_data();
  const std::vector<float>& event_float_block();
  std::string  event_blob_sum();

  void         next_event();
  void         add_skip_event (const std::string& event);
  std::string  file_type();
  int          file_version();

  GenericIn   *open_blob();
};

}

#endif /* SPECTMORPH_INFILE_HH */
