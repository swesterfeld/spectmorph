// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>

namespace SpectMorph
{

class NotifyBuffer
{
  std::vector<unsigned char> data;
  size_t rpos = 0;

  void
  write_simple (const void *ptr, size_t size)
  {
    // FIXME: this *may* occasionally allocate memory
    const unsigned char *raw_ptr = reinterpret_cast<const unsigned char *> (ptr);
    data.insert (data.end(), raw_ptr, raw_ptr + size);
  }
  void
  read_simple (void *ptr, size_t size)
  {
    unsigned char *raw_ptr = reinterpret_cast<unsigned char *> (ptr);
    std::copy (data.begin() + rpos, data.begin() + rpos + size, raw_ptr);
    rpos += size;
  }
public:
  void
  assign (NotifyBuffer& other)
  {
    data = other.data;
    rpos = 0;
  }
  void
  clear()
  {
    data.clear();
    rpos = 0;
  }
  void
  write_int (int i)
  {
    write_simple (&i, sizeof (i));
  }
  template<class T> void
  write_seq (const T* items, size_t length)
  {
    write_int (length);
    write_simple (items, length * sizeof (T));
  }
  size_t
  remaining()
  {
    return data.size() - rpos;
  }
  int
  read_int()
  {
    int i;
    read_simple (&i, sizeof (i));
    return i;
  }
  template<class T> std::vector<T>
  read_seq()
  {
    int seq_len = read_int();
    std::vector<T> result (seq_len);
    read_simple (result.data(), seq_len * sizeof (T));
    return result;
  }
};

}
