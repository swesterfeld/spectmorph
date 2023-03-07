// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include <stdio.h>
#include <assert.h>

#include <vector>

#include "smbinbuffer.hh"
#include "smmain.hh"
#include "smmidisynth.hh"

using namespace SpectMorph;
using std::vector;
using std::string;

class NotifyBuffer
{
  std::vector<unsigned char> data;
  size_t rpos = 0;

  void
  write_simple (const void *ptr, size_t size)
  {
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

static int VOICE_OP_VALUES_EVENT = 642137; // just some random number

struct MyVoiceOpValuesEvent : public SynthNotifyEvent
{
  struct Voice {
    uintptr_t voice;
    uintptr_t op;
    float     value;
  };
  MyVoiceOpValuesEvent (NotifyBuffer& notify_buffer) :
    voices (notify_buffer.read_seq<Voice>())
  {
  }
  std::vector<Voice> voices;
};

void
fill_notify_buffer (NotifyBuffer& buffer)
{
  static constexpr int N = 3;
  struct S
  {
    uintptr_t voice, op;
    float value;
  };
  S seq[N];

  for (int n = 0; n < N; n++)
    {
      seq[n].voice = (uintptr_t) &n;
      seq[n].op = (uintptr_t) &n;
      seq[n].value = n;
    }
  buffer.write_int (VOICE_OP_VALUES_EVENT);
  buffer.write_seq (seq, N);
}

MyVoiceOpValuesEvent *
create_event (NotifyBuffer& buffer)
{
  int type = buffer.read_int();
  if (type == VOICE_OP_VALUES_EVENT)
    {
      return new MyVoiceOpValuesEvent (buffer);
    }
  return nullptr;
}

void
perf (bool decode)
{
  vector<string> events;
  NotifyBuffer notify_buffer;

  const int RUNS = 10'000'000;
  const int EVENTS = 3;
  double t = get_time();
  for (int r = 0; r < RUNS; r++)
    {
      notify_buffer.clear();
      for (int i = 0; i < EVENTS; i++)
        {
          fill_notify_buffer (notify_buffer);
        }
      if (decode)
        {
          while (notify_buffer.remaining())
            {
              auto *e = create_event (notify_buffer);
              assert (e);
              delete e;
            }
        }
    }
  printf ("%.2f events/sec (decode = %s)\n", (EVENTS * RUNS) / (get_time() - t), decode ? "TRUE" : "FALSE");
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  perf (false);
  perf (true);
}
