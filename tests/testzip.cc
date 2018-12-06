#include <stdio.h>

#include <string>
#include <vector>

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_buf.h"
#include "mz_strm_split.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"

using std::string;
using std::vector;

class ZipReader
{
  void    *reader = nullptr;
  bool     need_close = false;
  int32_t  m_error = 0;
public:
  ZipReader (const string& filename)
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
  ~ZipReader()
  {
    if (need_close)
      mz_zip_reader_close (reader);

    if (reader)
      mz_zip_reader_delete (&reader);
  }
  vector<string>
  filenames()
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
  error() const
  {
    return m_error;
  }
  vector<uint8_t>
  read (const string& name)
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
};

class ZipWriter
{
  void    *writer = nullptr;
  bool     need_close = false;
  int32_t  m_error = 0;
public:
  ZipWriter (const string& filename)
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
  ~ZipWriter()
  {
    if (need_close)
      mz_zip_writer_close (writer);

    if (writer)
      mz_zip_writer_delete (&writer);
  }
  void
  add (const string& filename, const vector<uint8_t>& data)
  {
    if (m_error)
      return;

    mz_zip_file file_info = { 0, };

    file_info.version_madeby = MZ_VERSION_MADEBY;
    file_info.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
    file_info.filename = filename.c_str();
    file_info.uncompressed_size = data.size();

    m_error = mz_zip_writer_add_buffer (writer, const_cast<unsigned char *> (data.data()), data.size(), &file_info);
  }
  void
  add (const string& filename, const string& text)
  {
    const unsigned char *tbegin = reinterpret_cast<const unsigned char *> (text.data());
    const unsigned char *tend   = tbegin + text.size();
    vector<uint8_t> data (tbegin, tend);
    add (filename, data);
  }
  int32_t
  error() const
  {
    return m_error;
  }
};

int
main (int argc, char **argv)
{
  if (argc == 3 && strcmp (argv[1], "list") == 0)
    {
      ZipReader reader (argv[2]);

      for (auto name : reader.filenames())
        printf ("%s\n", name.c_str());

      if (reader.error())
        {
          printf ("ERR=%d\n", reader.error());
        }
    }
  if (argc == 3 && strcmp (argv[1], "get") == 0)
    {
      ZipReader reader (argv[2]);

      for (auto name : reader.filenames())
        {
          auto data = reader.read (name);

          printf ("%s [[[\n", name.c_str());
          for (auto ch : data)
            printf ("%c", ch);
          printf ("]]]\n\n");
        }
    }
  if (argc == 3 && strcmp (argv[1], "create") == 0)
    {
      ZipWriter writer (argv[2]);

      writer.add ("test.txt", "Hello World!\n");
      writer.add ("test2.txt", "Test II\n");

      if (writer.error())
        {
          printf ("ERR=%d\n", writer.error());
        }
    }
}
