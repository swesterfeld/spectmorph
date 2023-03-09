// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include <atomic>
#include <cassert>

namespace SpectMorph
{

class NotifyBuffer
{
  enum {
    STATE_EMPTY,
    STATE_DATA_VALID
  };
  std::atomic<int> state { STATE_EMPTY };
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
  bool
  start_write()
  {
    return state.load() == STATE_EMPTY;
  }
  void
  end_write()
  {
    state.store (STATE_DATA_VALID);
  }
  bool
  start_read()
  {
    if (state.load() == STATE_DATA_VALID)
      {
        rpos = 0;
        return true;
      }
    return false;
  }
  void
  end_read()
  {
    data.clear();
    state.store (STATE_EMPTY);
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
