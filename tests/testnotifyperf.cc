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

void
fill_notify_buffer (BinBuffer& buffer)
{
  static constexpr int N = 3;
  uintptr_t voice_seq[N];
  uintptr_t op_seq[N];
  float value_seq[N];

  for (int n = 0; n < N; n++)
    {
      voice_seq[n] = (uintptr_t) &n;
      op_seq[n] = (uintptr_t) &n;
      value_seq[n] = n;
    }
  buffer.write_start ("VoiceOpValuesEvent");
  buffer.write_ptr_seq (voice_seq, N);
  buffer.write_ptr_seq (op_seq, N);
  buffer.write_float_seq (value_seq, N);
  buffer.write_end();
}

VoiceOpValuesEvent *
create_event (const std::string& str)
{
  BinBuffer buffer;
  buffer.from_string (str);

  buffer.read_int();
  const char *type = buffer.read_string_inplace();
  if (strcmp (type, "VoiceOpValuesEvent") == 0)
    {
      VoiceOpValuesEvent *v = new VoiceOpValuesEvent();

      buffer.read_ptr_seq (v->voice);
      buffer.read_ptr_seq (v->op);
      buffer.read_float_seq (v->value);

      if (buffer.read_error())
        {
          printf ("error reading %s event\n", type);
          delete v;
          return nullptr;
        }
      return v;
    }
  return nullptr;
}

void
perf (bool decode)
{
  vector<string> events;
  BinBuffer notify_buffer;

  const int RUNS = 100000;
  const int EVENTS = 3;
  double t = get_time();
  for (int r = 0; r < RUNS; r++)
    {
      events.clear();
      for (int i = 0; i < EVENTS; i++)
        {
          fill_notify_buffer (notify_buffer);
          events.push_back (notify_buffer.to_string());
        }
      if (decode)
        {
          for (const auto& str : events)
            {
              auto *e = create_event (str);
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
