// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_BINBUFFER_HH
#define SPECTMORPH_BINBUFFER_HH

#include "smhexstring.hh"

#include <vector>

namespace SpectMorph
{

class BinBuffer
{
  std::vector<unsigned char> data;

  size_t rpos = 0;
  bool   m_read_error = false;
public:
  void
  write_start (const char *id)
  {
    data.clear();

    write_int (0); // reserved for length
    write_string (id);
  }
  void
  write_end()
  {
    size_t len = data.size();

    data[0] = (len >> 24);
    data[1] = (len >> 16);
    data[2] = (len >> 8);
    data[3] = len;
  }
  void
  write_int (int i)
  {
    write_byte (i >> 24);
    write_byte (i >> 16);
    write_byte (i >> 8);
    write_byte (i);
  }
  void
  write_float (float f)
  {
    union {
      float f;
      int i;
    } u;
    u.f = f;
    write_int (u.i);
  }
  void
  write_string (const char *str)
  {
    const unsigned char *raw_str = reinterpret_cast<const unsigned char *> (str);
    size_t len = strlen (str) + 1;

    write_int (len);
    data.insert (data.end(), raw_str, raw_str + len);
  }
  void
  write_byte (unsigned char byte)
  {
    data.push_back (byte);
  }
  void
  write_int_seq (const int* ints, size_t length)
  {
    write_int (length);

    for (size_t i = 0; i < length; i++)
      write_int (ints[i]);
  }
  void
  write_float_seq (const float* floats, size_t length)
  {
    write_int (length);

    for (size_t i = 0; i < length; i++)
      write_float (floats[i]);
  }
  std::string
  to_string()
  {
    std::string result;
    char hex[17] = "0123456789abcdef";

    for (unsigned char byte : data)
      {
        result += hex[(byte >> 4) & 0xf];
        result += hex[byte & 0xf];
      }
    return result;
  }
  bool
  from_string (const std::string& str)
  {
    data.clear();
    return HexString::decode (str, data);
  }
  size_t
  remaining()
  {
    return data.size() - rpos;
  }
  bool
  read_error()
  {
    return m_read_error;
  }
  int
  read_int()
  {
    if (remaining() >= 4)
      {
        int result = (data[rpos]     << 24)
                   + (data[rpos + 1] << 16)
                   + (data[rpos + 2] << 8)
                   +  data[rpos + 3];
        rpos += 4;

        return result;
      }
    else
      {
        m_read_error = true;
        return 0;
      }
  }
  float
  read_float()
  {
    union {float f; int i; } u;
    u.i = read_int();

    if (!m_read_error)
      return u.f;
    else
      return 0.0;
  }
  const char *
  read_string_inplace()
  {
    int   len = read_int();
    char *data = (char *)read (len);

    if (data && len)
      return data;
    else
      return "";
  }
  void *
  read (size_t l)
  {
    if (l >= 0 && remaining() >= l)
      {
        void *result = &data[rpos];
        rpos += l;
        return result;
      }
    else
      {
        m_read_error = true;
        return nullptr;
      }
  }
  void
  read_int_seq (std::vector<int>& result)
  {
    // might be optimizable a bit
    size_t seqlen = read_int();

    if (seqlen * 4 >= 0 && remaining() >= seqlen * 4)
      {
        result.resize (seqlen);

        for (size_t i = 0; i < seqlen; i++)
          result[i] = read_int();
      }
    else
      {
        m_read_error = true;
        result.clear();
      }
  }
  void
  read_float_seq (std::vector<float>& result)
  {
    // might be optimizable a bit
    size_t seqlen = read_int();

    if (seqlen * 4 >= 0 && remaining() >= seqlen * 4)
      {
        result.resize (seqlen);

        for (size_t i = 0; i < seqlen; i++)
          result[i] = read_float();
      }
    else
      {
        m_read_error = true;
        result.clear();
      }
  }
};

}

#endif

