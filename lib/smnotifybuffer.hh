// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include <atomic>
#include <cassert>
#include <cstring>

namespace SpectMorph
{

/*
 * NotifyBuffer provides a method to transport notification events from the DSP
 * thread to the UI thread without locks. It also avoids memory allocations in
 * the DSP thread.
 *
 * To do this, it uses a std::atomic which indicates
 *  - that the DSP thread can write the buffer (STATE_EMPTY)
 *  - that the UI thread can read the buffer (STATE_DATA_VALID)
 *  - that the DSP thread failed to write the data because the available space
 *    was too small, and that the UI thread should allocate more memory (STATE_NEED_RESIZE)
 *
 * There is no guarantee that each event is delivered, because writing can fail
 *  - if the UI thread didn't read the old events from the buffer yet
 *  - if the available space is too small to write all events
 */
class NotifyBuffer
{
  enum {
    STATE_EMPTY,
    STATE_DATA_VALID,
    STATE_NEED_RESIZE
  };
  std::atomic<int> state { STATE_EMPTY };
  std::vector<unsigned char> data;
  size_t rpos = 0;
  size_t wpos = 0;

  void
  write_simple (const void *ptr, size_t size) // DSP thread
  {
    size_t new_wpos = wpos + size;
    if (new_wpos <= data.size())
      memcpy (&data[wpos], ptr, size);

    wpos = new_wpos;
  }
  void
  read_simple (void *ptr, size_t size) // UI thread
  {
    if (size)
      {
        memcpy (ptr, &data[rpos], size);
        rpos += size;
      }
  }
public:
  NotifyBuffer() :
    data (32)
  {
  }
  bool
  start_write() // DSP thread
  {
    if (state.load() == STATE_EMPTY)
      {
        wpos = 0;
        return true;
      }
    return false;
  }
  void
  end_write() // DSP thread
  {
    if (wpos <= data.size())
      {
        state.store (STATE_DATA_VALID);
      }
    else
      {
        state.store (STATE_NEED_RESIZE);
      }
  }
  void
  resize_if_necessary() // UI thread
  {
    if (state.load() == STATE_NEED_RESIZE)
      {
        data.resize (data.size() * 2);
        state.store (STATE_EMPTY);
      }
  }
  bool
  start_read() // UI thread
  {
    if (state.load() == STATE_DATA_VALID)
      {
        rpos = 0;
        return true;
      }
    return false;
  }
  void
  end_read() // UI thread
  {
    state.store (STATE_EMPTY);
  }
  void
  write_int (int i) // DSP thread
  {
    write_simple (&i, sizeof (i));
  }
  template<class T> void
  write_seq (const T* items, size_t length) // DSP thread
  {
    write_int (length);
    write_simple (items, length * sizeof (T));
  }
  size_t
  remaining() // UI thread
  {
    return wpos - rpos;
  }
  int
  read_int() // UI thread
  {
    int i;
    read_simple (&i, sizeof (i));
    return i;
  }
  template<class T> std::vector<T>
  read_seq() // UI thread
  {
    int seq_len = read_int();
    std::vector<T> result (seq_len);
    read_simple (result.data(), seq_len * sizeof (T));
    return result;
  }
};

}
