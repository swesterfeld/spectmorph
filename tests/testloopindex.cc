// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smaudio.hh"
#include "smmain.hh"
#include "smlivedecoder.hh"

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);

  Audio a;
  printf ("forward loop (5-10): ");
  a.loop_type = Audio::LOOP_FRAME_FORWARD;
  a.loop_start = 5;
  a.loop_end   = 10;
  for (size_t i = 0; i < 30; i++)
    {
      printf ("%zd ", LiveDecoder::compute_loop_frame_index (i, &a));
    }
  printf ("\n");

  printf ("ping-pong loop (5-10): ");
  a.loop_type = Audio::LOOP_FRAME_PING_PONG;
  a.loop_start = 5;
  a.loop_end   = 10;
  for (size_t i = 0; i < 30; i++)
    {
      printf ("%zd ", LiveDecoder::compute_loop_frame_index (i, &a));
    }
  printf ("\n");

  printf ("forward loop (5-5): ");
  a.loop_type = Audio::LOOP_FRAME_FORWARD;
  a.loop_start = 5;
  a.loop_end   = 5;
  for (size_t i = 0; i < 30; i++)
    {
      printf ("%zd ", LiveDecoder::compute_loop_frame_index (i, &a));
    }
  printf ("\n");

  printf ("ping-pong loop (5-5): ");
  a.loop_type = Audio::LOOP_FRAME_PING_PONG;
  a.loop_start = 5;
  a.loop_end   = 5;
  for (size_t i = 0; i < 30; i++)
    {
      printf ("%zd ", LiveDecoder::compute_loop_frame_index (i, &a));
    }
  printf ("\n");

  printf ("forward loop (5-6): ");
  a.loop_type = Audio::LOOP_FRAME_FORWARD;
  a.loop_start = 5;
  a.loop_end   = 6;
  for (size_t i = 0; i < 30; i++)
    {
      printf ("%zd ", LiveDecoder::compute_loop_frame_index (i, &a));
    }
  printf ("\n");

  printf ("ping-pong loop (5-6): ");
  a.loop_type = Audio::LOOP_FRAME_PING_PONG;
  a.loop_start = 5;
  a.loop_end   = 6;
  for (size_t i = 0; i < 30; i++)
    {
      printf ("%zd ", LiveDecoder::compute_loop_frame_index (i, &a));
    }
  printf ("\n");

  return 0;
}
