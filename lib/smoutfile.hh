// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPECTMORPH_OUT_FILE_HH
#define SPECTMORPH_OUT_FILE_HH

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <set>
#include "smgenericout.hh"

namespace SpectMorph
{

class MorphOperatorPtr;

class OutFile
{
  GenericOut           *file;
  bool                  delete_file;
  std::set<std::string> stored_blobs;

protected:
  void write_raw_string (const std::string& s);
  void write_raw_int (int i);
  void write_file_type_and_version (const std::string& file_type, int file_version);

public:
  OutFile (const std::string& filename, const std::string& file_type, int file_version);
  OutFile (GenericOut *outfile, const std::string& file_type, int file_version);

  bool
  open_ok()
  {
    return file != NULL;
  }
  ~OutFile();

  void begin_section (const std::string& s);
  void end_section();

  void write_bool (const std::string& s, bool b);
  void write_int (const std::string& s, int i);
  void write_string (const std::string& s, const std::string& data);
  void write_float (const std::string& s, double f);
  void write_float_block (const std::string& s, const std::vector<float>& fb);
  void write_uint16_block (const std::string& s, const std::vector<uint16_t>& ib);
  void write_blob (const std::string& s, const void *data, size_t size);
  void write_operator (const std::string& name, const MorphOperatorPtr& op);
};

}

#endif /* SPECTMORPH_OUT_FILE_HH */
