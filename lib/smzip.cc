// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smzip.hh"
#include "smutils.hh"

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_buf.h"
#include "mz_strm_split.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"
#include "mz_strm_mem.h"

using std::string;
using std::vector;

using namespace SpectMorph;

ZipReader::ZipReader (const string& filename)
{
  void *ptr = mz_zip_reader_create (&reader);
  if (!ptr)
    {
      m_error = MZ_MEM_ERROR;
      return;
    }
  m_error = mz_zip_reader_open_file (reader, filename.c_str());
  if (m_error)
    return;

  need_close = true;
}

ZipReader::ZipReader (const std::vector<uint8_t>& data) :
  m_data (data)
{
  mz_stream_mem_create (&read_mem_stream);
  mz_stream_mem_set_buffer (read_mem_stream, (void *)&m_data[0], m_data.size());
  mz_stream_open (read_mem_stream, NULL, MZ_OPEN_MODE_READ);

  void *ptr = mz_zip_reader_create (&reader);
  if (!ptr)
    {
      m_error = MZ_MEM_ERROR;
      return;
    }
 m_error = mz_zip_reader_open (reader, read_mem_stream);
 if (m_error)
   return;

  need_close = true;
}

ZipReader::~ZipReader()
{
  if (need_close)
    mz_zip_reader_close (reader);

  if (reader)
    mz_zip_reader_delete (&reader);

  if (read_mem_stream)
    {
      mz_stream_mem_close (read_mem_stream);
      mz_stream_mem_delete (&read_mem_stream);
      read_mem_stream = nullptr;
    }
}

vector<string>
ZipReader::filenames()
{
  if (m_error)
    return {};

  m_error = mz_zip_reader_goto_first_entry (reader);
  if (m_error)
    return {};

  vector<string> names;
  do
    {
      mz_zip_file *file_info = nullptr;

      m_error = mz_zip_reader_entry_get_info (reader, &file_info);
      if (m_error)
        return {};

      names.push_back (file_info->filename);

      int err = mz_zip_reader_goto_next_entry (reader);
      if (err == MZ_END_OF_LIST)
        {
          /* success */
          return names;
        }
      else
        {
          m_error = err;
        }
    }
  while (m_error == MZ_OK);

  return {}; // error while reading entries
}

int32_t
ZipReader::error() const
{
  return m_error;
}

vector<uint8_t>
ZipReader::read (const string& name)
{
  mz_zip_file *file_info = nullptr;

  if (m_error)
    return {};

  m_error = mz_zip_reader_locate_entry (reader, name.c_str(), false);
  if (m_error)
    return {};

  m_error = mz_zip_reader_entry_get_info (reader, &file_info);
  if (m_error)
    return {};

  m_error = mz_zip_reader_entry_open (reader);
  if (m_error)
    return {};

  vector<uint8_t> result (file_info->uncompressed_size);

  int32_t read = mz_zip_reader_entry_read (reader, &result[0], result.size());
  if (read < 0)
    {
      m_error = read;
      return {};
    }
  return result;
}

ZipWriter::ZipWriter (const string& filename)
{
  void *ptr = mz_zip_writer_create (&writer);
  if (!ptr)
    {
      m_error = MZ_MEM_ERROR;
      return;
    }

  m_error = mz_zip_writer_open_file (writer, filename.c_str(), /* disk size */ 0, /* append */ 0);
  if (m_error)
    return;

  need_close = true;
}

ZipWriter::ZipWriter()
{
  mz_stream_mem_create (&write_mem_stream);
  mz_stream_mem_set_grow_size (write_mem_stream, 256 * 1024);
  mz_stream_open (write_mem_stream, nullptr, MZ_OPEN_MODE_CREATE);

  void *ptr = mz_zip_writer_create (&writer);
  if (!ptr)
    {
      m_error = MZ_MEM_ERROR;
      return;
    }

  m_error = mz_zip_writer_open (writer, write_mem_stream);
  if (m_error)
    return;

  need_close = true;
}

vector<uint8_t>
ZipWriter::data()
{
  if (need_close)
    {
      mz_zip_writer_close (writer);
      need_close = false;
    }

  if (write_mem_stream)
    {
      const uint8_t *buffer_ptr = NULL;

      mz_stream_mem_get_buffer (write_mem_stream, (const void **)&buffer_ptr);
      mz_stream_mem_seek (write_mem_stream, 0, MZ_SEEK_END);
      size_t buffer_size = (int32_t) mz_stream_mem_tell (write_mem_stream);

      return std::vector<uint8_t> (buffer_ptr, buffer_ptr + buffer_size);
    }
  return {};
}

ZipWriter::~ZipWriter()
{
  if (need_close)
    mz_zip_writer_close (writer);

  if (writer)
    mz_zip_writer_delete (&writer);

  if (write_mem_stream)
    {
      mz_stream_mem_close (write_mem_stream);
      mz_stream_mem_delete (&write_mem_stream);
      write_mem_stream = nullptr;
    }
}

void
ZipWriter::add (const string& filename, const vector<uint8_t>& data, Compress compress)
{
  if (m_error)
    return;

  mz_zip_file file_info = { 0, };

  file_info.version_madeby = MZ_VERSION_MADEBY;
  switch (compress)
  {
    case Compress::STORE:   file_info.compression_method = MZ_COMPRESS_METHOD_STORE;
                            break;
    case Compress::DEFLATE: file_info.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
                            break;
  }
  file_info.filename = filename.c_str();
  file_info.uncompressed_size = data.size();
#ifdef SM_OS_LINUX
  /* set sane unix specific attribute bits */
  file_info.external_fa = 0664 << 16;
#endif

  m_error = mz_zip_writer_add_buffer (writer, const_cast<unsigned char *> (data.data()), data.size(), &file_info);
}

void
ZipWriter::add (const string& filename, const string& text, Compress compress)
{
  const unsigned char *tbegin = reinterpret_cast<const unsigned char *> (text.data());
  const unsigned char *tend   = tbegin + text.size();
  vector<uint8_t> data (tbegin, tend);
  add (filename, data, compress);
}

int32_t
ZipWriter::error() const
{
  return m_error;
}
