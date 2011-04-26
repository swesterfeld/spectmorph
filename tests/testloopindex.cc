/*
 * Copyright (C) 2011 Stefan Westerfeld
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "smaudio.hh"
#include "smmain.hh"
#include "smlivedecoder.hh"

using namespace SpectMorph;

int
main (int argc, char **argv)
{
  sm_init (&argc, &argv);

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
