// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl-2.1.html

#include "smmain.hh"
#include "sminfile.hh"
#include "smoutfile.hh"
#include "smmemout.hh"
#include "smstdiosubin.hh"

#include <cassert>
#include <climits>
#include <cstdio>

using namespace SpectMorph;
using std::vector;

static vector<unsigned char>
header()
{
  vector<unsigned char> data;
  {
    OutFile out (MemOut::open (&data), "test", 1);
  }
  data.pop_back(); // remove EOF marker
  return data;
}

static void
append_int (vector<unsigned char>& data, int value)
{
  uint32_t bits = value;
  for (int i = 0; i < 4; i++)
    data.push_back ((bits >> (8 * i)) & 0xff);
}

static void
append_block (vector<unsigned char>& data, char type, int count)
{
  data.insert (data.end(), { static_cast<unsigned char> (type), 'x', 0 });
  append_int (data, count);
}

static GenericInP
open_input (const vector<unsigned char>& data, bool mapped)
{
  if (mapped)
    return MMapIn::open_vector (data);

  FILE *file = fopen ("testinfile.out", "wb");
  assert (file);
  assert (fwrite (data.data(), 1, data.size(), file) == data.size());
  assert (fclose (file) == 0);
  auto input = StdioIn::open ("testinfile.out");
  size_t remaining;
  assert (input && input->get_remaining (remaining) && remaining == data.size());
  // Substream queries must respect both the physical file and logical bounds.
  auto sub = StdioSubIn::open ("testinfile.out", 1, data.size() + 100);
  assert (sub && sub->get_remaining (remaining) && remaining == data.size() - 1);
  sub = StdioSubIn::open ("testinfile.out", 1, 1);
  assert (sub && sub->get_remaining (remaining) && remaining == 1);
  assert (sub->get_byte() != EOF);
  assert (sub->get_remaining (remaining) && remaining == 0);
  return input;
}

static void
check_error (const vector<unsigned char>& data, bool mapped, bool skip)
{
  InFile in (open_input (data, mapped));
  if (skip)
    in.add_skip_event ("x");
  assert (in.event() == InFile::READ_ERROR);
  // A tiny malformed input must not allocate its advertised payload size.
  assert (in.event_float_block().capacity() <= 4096);
  assert (in.event_uint16_block().capacity() <= 4096);
}

static void
check_skip (GenericInP input, size_t length)
{
  assert (input && length > 0);
  assert (input->skip (1));
  assert (input->get_pos() == 1);
  assert (!input->skip (SIZE_MAX));
  assert (!input->skip (length));
  assert (input->get_pos() == 1);
  assert (input->skip (length - 1));
  assert (input->get_pos() == length);
  assert (input->skip (0));
  assert (!input->skip (1));
  assert (input->get_pos() == length);
}

static void
check_read_position (GenericInP input, size_t length)
{
  assert (input && length > 0);
  assert (input->get_pos() == 0);
  assert (input->get_byte() != EOF);
  assert (input->get_pos() == 1);
  size_t remaining;
  assert (input->get_remaining (remaining) && remaining == length - 1);
  char buffer[8];
  assert (length < sizeof (buffer));
  assert (input->read (buffer, sizeof (buffer)) == int (length - 1));
  assert (input->get_pos() == length);
  assert (input->get_remaining (remaining) && remaining == 0);
  for (int i = 0; i < 2; i++)
    {
      assert (input->get_byte() == EOF);
      assert (input->read (buffer, 1) <= 0);
      assert (input->get_pos() == length);
      assert (input->get_remaining (remaining) && remaining == 0);
    }
}

int
main (int argc, char **argv)
{
  Main main (&argc, &argv);
  assert (!StdioIn::open ("."));
  assert (!StdioSubIn::open (".", 0, 0));
#ifndef _WIN32
  assert (!StdioIn::open ("/dev/null"));
  assert (!StdioSubIn::open ("/dev/null", 0, 0));
#endif
  {
    FILE *file = fopen ("testinfile.out", "wb");
    assert (file && fclose (file) == 0);
    assert (!StdioSubIn::open ("testinfile.out", SIZE_MAX, 0));
    auto input = StdioIn::open ("testinfile.out");
    auto sub = StdioSubIn::open ("testinfile.out", 0, 0);
    size_t remaining;
    assert (input && input->get_remaining (remaining) && remaining == 0);
    assert (sub && sub->get_remaining (remaining) && remaining == 0);
  }
  {
    vector<unsigned char> data { 'a', 'b', 'c' };
    check_read_position (open_input (data, false), data.size());
    check_read_position (StdioSubIn::open ("testinfile.out", 1, 100), 2);
    check_skip (open_input (data, false), data.size());
    check_skip (open_input (data, true), data.size());
    check_skip (StdioSubIn::open ("testinfile.out", 1, 100), 2); // physical end
    check_skip (StdioSubIn::open ("testinfile.out", 1, 1), 1); // logical end
    vector<unsigned char> empty;
    auto input = MMapIn::open_vector (empty);
    assert (input->skip (0));
    assert (!input->skip (1));
    assert (!input->skip (SIZE_MAX));
  }
  for (bool mapped : { false, true })
    {
      for (char type : { 'F', '6' })
        {
          const int element_size = type == 'F' ? 4 : 2;
          for (int count : { INT_MIN, -2, -1, 1, INT_MAX / element_size,
                             INT_MAX / element_size + 1, INT_MAX })
            {
              auto data = header();
              append_block (data, type, count);
              data.push_back (0); // truncated payload
              for (bool skip : { false, true })
                check_error (data, mapped, skip);
            }
          auto partial_count = header();
          partial_count.insert (partial_count.end(), { static_cast<unsigned char> (type), 'x', 0, 1, 0 });
          for (bool skip : { false, true })
            check_error (partial_count, mapped, skip);
        }

      for (int size : { INT_MIN, -2, 1, INT_MAX })
        {
          auto data = header();
          append_block (data, 'O', size);
          data.push_back (0); // checksum, but no payload
          check_error (data, mapped, false);
        }
      for (int size : { -1, 0, 3 })
        {
          auto data = header();
          append_block (data, 'O', size);
          data.push_back (0);
          if (size > 0)
            data.insert (data.end(), { 'a', 'b', 'c' });
          data.push_back ('Z');
          InFile in (open_input (data, mapped));
          assert (in.event() == (size == -1 ? InFile::BLOB_REF : InFile::BLOB));
          if (size > 0)
            {
              auto blob = in.open_blob();
              char bytes[3];
              assert (blob && blob->read (bytes, 3) == 3);
              assert (bytes[0] == 'a' && bytes[1] == 'b' && bytes[2] == 'c');
            }
          in.next_event();
          assert (in.event() == InFile::END_OF_FILE);
        }

      // Valid blocks include both empty and larger payloads.
      for (size_t count : { size_t (0), size_t (4097) })
        {
          vector<float> floats (count, 1.25f);
          vector<uint16_t> ints (count, 0xabcd);
          vector<unsigned char> data;
          {
            OutFile out (MemOut::open (&data), "test", 1);
            out.write_float_block ("x", floats);
            out.write_uint16_block ("x", ints);
            out.write_int ("marker", 42);
          }
          for (bool skip : { false, true })
            {
              InFile in (open_input (data, mapped));
              if (skip)
                in.add_skip_event ("x");
              else
                {
                  assert (in.event() == InFile::FLOAT_BLOCK);
                  assert (in.event_float_block() == floats);
                  in.next_event();
                  assert (in.event() == InFile::UINT16_BLOCK);
                  assert (in.event_uint16_block() == ints);
                  in.next_event();
                }
              assert (in.event() == InFile::INT && in.event_int() == 42);
              in.next_event();
              assert (in.event() == InFile::END_OF_FILE);
            }
        }

      auto data = header();
      for (int i = 0; i < 100000; i++)
        append_block (data, i % 2 ? 'F' : '6', 0);
      data.push_back ('Z');
      InFile in (open_input (data, mapped));
      in.add_skip_event ("x");
      assert (in.event() == InFile::END_OF_FILE);
    }
  assert (remove ("testinfile.out") == 0);
}
