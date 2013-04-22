// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "sminfile.hh"
#include <assert.h>
#include <glib.h>

using std::string;
using std::vector;
using SpectMorph::InFile;
using SpectMorph::GenericIn;

/**
 * Create InFile object for reading a file.
 *
 * \param filename name of the file
 */
InFile::InFile (const string& filename) :
  file_delete (true)
{
  file = GenericIn::open (filename);
  current_event = NONE;

  read_file_type_and_version();
}

/**
 * Create InFile object for reading an input stream.
 *
 * \param file the input stream object to read data from
 */
InFile::InFile (GenericIn *file) :
  file (file),
  file_delete (false)
{
  current_event = NONE;
  read_file_type_and_version();
}

InFile::~InFile()
{
  if (file && file_delete)
    {
      delete file;
      file = NULL;
    }
}

void
InFile::read_file_type_and_version()
{
  if (file)
    {
      if (file->get_byte() == 'T')
        if (read_raw_string (m_file_type))
          if (file->get_byte() == 'V')
            if (read_raw_int (m_file_version))
              return;
    }
  m_file_type = "unknown";
  m_file_version = 0;
}

/**
 * Get current event type.
 *
 * \returns current event type (or READ_ERROR or END_OF_FILE).
 */
InFile::Event
InFile::event()
{
  if (current_event == NONE)
    next_event();

  return current_event;
}

bool
InFile::read_raw_string (string& str)
{
  size_t remaining;
  unsigned char *mem = file->mmap_mem (remaining);
  if (mem) /* fast variant of reading strings for the mmap case */
    {
      for (size_t i = 0; i < remaining; i++)
        {
          if (mem[i] == 0)
            {
              if (file->skip (i + 1))
                {
                  str.assign (reinterpret_cast <char *> (mem), i);
                  return true;
                }
            }
        }
    }

  str.clear();

  int c;
  while ((c = file->get_byte()) > 0)
    str += c;

  if (c == 0)
    return true;
  return false;
}

/**
 * Reads next event from file. Call event() to get event type, and event_*() to get event data.
 */
void
InFile::next_event()
{
  int c = file->get_byte();

  if (c == 'Z')  // eof
    {
      if (file->get_byte() == EOF)   // Z needs to be followed by EOF
        current_event = END_OF_FILE;
      else                           // Z and more stuff is an error
        current_event = READ_ERROR;
      return;
    }
  else if (c == 'B')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        current_event = BEGIN_SECTION;
    }
  else if (c == 'E')
    {
      current_event = END_SECTION;
    }
  else if (c == 'f')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        if (read_raw_float (current_event_float))
          current_event = FLOAT;
    }
  else if (c == 'i')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        if (read_raw_int (current_event_int))
          current_event = INT;
    }
  else if (c == 'b')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        if (read_raw_bool (current_event_bool))
          current_event = BOOL;
    }
  else if (c == 's')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        if (read_raw_string (current_event_data))
          current_event = STRING;
    }
  else if (c == 'F')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        {
          if (skip_events.find (current_event_str) != skip_events.end())
            {
              if (skip_raw_float_block())
                {
                  next_event();
                  return;
                }
            }
          else
            {
              if (read_raw_float_block (current_event_float_block))
                current_event = FLOAT_BLOCK;
            }
        }
    }
  else if (c == '6') // 16bit block
    {
      current_event = READ_ERROR;

      if (read_raw_string (current_event_str))
        {
          if (skip_events.find (current_event_str) != skip_events.end())
            {
              if (skip_raw_uint16_block())
                {
                  next_event();
                  return;
                }
            }
          else
            {
              if (read_raw_uint16_block (current_event_uint16_block))
                current_event = UINT16_BLOCK;
            }
        }
    }
  else if (c == 'O')
    {
      current_event = READ_ERROR;
      if (read_raw_string (current_event_str))
        {
          int blob_size;
          if (read_raw_int (blob_size))
            {
              string blob_sum;
              if (read_raw_string (blob_sum))
                {
                  if (blob_size == -1)
                    {
                      current_event = BLOB_REF;
                      current_event_blob_sum = blob_sum;
                    }
                  else
                    {
                      int blob_pos = file->get_pos();
                      if (file->skip (blob_size)) // skip actual blob data
                        {
                          current_event = BLOB;
                          current_event_blob_size = blob_size;
                          current_event_blob_pos  = blob_pos;
                          current_event_blob_sum  = blob_sum;
                        }
                    }
                }
            }
        }
    }
  else
    {
      current_event = READ_ERROR;
    }
}

bool
InFile::read_raw_int (int& i)
{
  if (file->read (&i, 4) == 4)
    {
      // little endian encoding
      i = GINT32_FROM_LE (i);
      return true;
    }
  else
    {
      return false;
    }
}

bool
InFile::read_raw_bool (bool& b)
{
  char bchar;
  if (file->read (&bchar, 1) == 1)
    {
      if (bchar == 0)
        {
          b = false;
          return true;
        }
      else if (bchar == 1)
        {
          b = true;
          return true;
        }
    }
  return false;
}

bool
InFile::read_raw_float (float &f)
{
  union {
    float f;
    int i;
  } u;
  bool result = read_raw_int (u.i);
  f = u.f;
  return result;
}

bool
InFile::read_raw_float_block (vector<float>& fb)
{
  int size;
  if (!read_raw_int (size))
    return false;

  fb.resize (size);
  int *buffer = reinterpret_cast <int*> (&fb[0]);

  if (file->read (&buffer[0], fb.size() * 4) != size * 4)
    return false;

#if G_BYTE_ORDER != G_LITTLE_ENDIAN
  for (size_t x = 0; x < fb.size(); x++)
    buffer[x] = GINT32_FROM_LE (buffer[x]);
#endif
  return true;
}

bool
InFile::read_raw_uint16_block (vector<uint16_t>& ib)
{
  int size;
  if (!read_raw_int (size))
    return false;

  ib.resize (size);

  if (file->read (&ib[0], ib.size() * 2) != size * 2)
    return false;

#if G_BYTE_ORDER != G_LITTLE_ENDIAN
  for (size_t x = 0; x < ib.size(); x++)
    ib[x] = GUINT16_FROM_LE (ib[x]);
#endif
  return true;
}

bool
InFile::skip_raw_float_block()
{
  int size;
  if (!read_raw_int (size))
    return false;

  return file->skip (size * 4);
}

bool
InFile::skip_raw_uint16_block()
{
  int size;
  if (!read_raw_int (size))
    return false;

  return file->skip (size * 2);
}

/**
 * This function will open the blob (it will only work for BLOB events, not BLOB_REF events)
 * and the returned GenericIn object be used to read the content of the BLOB. The caller
 * is responsible for freeing the GenericIn object when done. NULL will be returned on
 * error.
 */
GenericIn *
InFile::open_blob()
{
  return file->open_subfile (current_event_blob_pos, current_event_blob_size);
}

/**
 * Get name of the current event.
 *
 * \returns current event name
 */
string
InFile::event_name()
{
  return current_event_str;
}

/**
 * Get float data of the current event (only if the event is a FLOAT).
 *
 * \returns current event float data.
 */
float
InFile::event_float()
{
  return current_event_float;
}

/**
 * Get int data of the current event (only if the event is an INT).
 *
 * \returns current event int data
 */
int
InFile::event_int()
{
  return current_event_int;
}

/**
 * Get bool data of the current event (only if the event is BOOL).
 *
 * \returns current event bool data
 */
bool
InFile::event_bool()
{
  return current_event_bool;
}

/**
 * Get string data of the current event (only if the event is STRING).
 *
 * \returns current event string data
 */
string
InFile::event_data()
{
  return current_event_data;
}

/**
 * Get float block data of the current event (only if the event is FLOAT_BLOCK).
 *
 * \returns current event float block data (by reference)
 */
const vector<float>&
InFile::event_float_block()
{
  return current_event_float_block;
}

/**
 * Get uint16 block data of the current event (only if the event is UINT16_BLOCK).
 *
 * \returns current event uint16 block data (by reference)
 */
const vector<uint16_t>&
InFile::event_uint16_block()
{
  return current_event_uint16_block;
}

/**
 * Get blob's checksum.  This works for both: BLOB objects and BLOB_REF
 * objects.  During writing files, the first occurence of a BLOB is stored
 * completely, whereas after that, only the blob sum is stored as BLOB_REF
 * event. During loading, code for handling both needs to be supplied.
 *
 * \returns the blob's checksum
 */
string
InFile::event_blob_sum()
{
  return current_event_blob_sum;
}

/**
 * Add event names to skip (currently only implemented for FLOAT_BLOCK events); this
 * speeds up reading files, while ignoring certain events.
 *
 * \param skip_event name of the event to skip
 */
void
InFile::add_skip_event (const string& skip_event)
{
  skip_events.insert (skip_event);
}

/**
 * Get file type (usually a class name, like "SpectMorph::WavSet").
 *
 * \returns file type
 */
string
InFile::file_type()
{
  return m_file_type;
}

/**
 * Get file version, an integer.
 *
 * \returns file version
 */
int
InFile::file_version()
{
  return m_file_version;
}
