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

#ifndef SPECTMORPH_OUT_FILE_HH
#define SPECTMORPH_OUT_FILE_HH

#include <stdio.h>
#include <string>
#include <vector>

namespace SpectMorph
{

class OutFile
{
  FILE *file;
protected:
  void write_raw_string (const std::string& s);
  void write_raw_int (int i);

public:
  OutFile (const std::string& filename)
  {
    file = fopen (filename.c_str(), "w");
  }
  bool
  open_ok()
  {
    return file != NULL;
  }
  ~OutFile();

  void begin_section (const std::string& s);
  void end_section();

  void write_int (const std::string& s, int i);
  void write_string (const std::string& s, const std::string& data);
  void write_float (const std::string& s, double f);
  void write_float_block (const std::string& s, const std::vector<float>& fb);
  void write_blob (const std::string& s, const void *data, size_t size);
};

}

#endif /* SPECTMORPH_OUT_FILE_HH */
