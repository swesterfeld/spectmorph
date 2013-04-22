// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smaudio.hh"
#include "smoutfile.hh"
#include "sminfile.hh"
#include "smstdioout.hh"
#include "smleakdebugger.hh"
#include "smmemout.hh"
#include "smmmapin.hh"
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <bse/bse.hh>

using std::string;
using std::vector;

using namespace SpectMorph;

static LeakDebugger leak_debugger ("SpectMorph::Audio");

/**
 * This function loads a SM-File.
 *
 * \param filename the name of the SM-File to be loaded
 * \param load_options specify whether to load or skip debug information
 * \returns a BseErrorType indicating whether loading was successful
 */
BseErrorType
SpectMorph::Audio::load (const string& filename, AudioLoadOptions load_options)
{
  GenericIn *file = GenericIn::open (filename);
  if (!file)
    return BSE_ERROR_FILE_NOT_FOUND;

  BseErrorType result = load (file, load_options);
  delete file;

  return result;
}

BseErrorType
SpectMorph::Audio::load (GenericIn *file, AudioLoadOptions load_options)
{
  SpectMorph::AudioBlock *audio_block = NULL;

  InFile ifile (file);

  string section;
  size_t contents_pos = 0; /* init to get rid of gcc warning */

  if (!ifile.open_ok())
    return BSE_ERROR_FILE_NOT_FOUND;

  if (ifile.file_type() != "SpectMorph::Audio")
    return BSE_ERROR_FORMAT_INVALID;

  if (ifile.file_version() != SPECTMORPH_BINARY_FILE_VERSION)
    return BSE_ERROR_FORMAT_INVALID;

  if (load_options == AUDIO_SKIP_DEBUG)
    {
      ifile.add_skip_event ("original_fft");
      ifile.add_skip_event ("debug_samples");
    }

  while (ifile.event() != InFile::END_OF_FILE)
    {
      if (ifile.event() == InFile::BEGIN_SECTION)
        {
          assert (section == "");
          section = ifile.event_name();

          if (section == "frame")
            {
              assert (audio_block == NULL);
              assert (contents_pos < contents.size());

              audio_block = &contents[contents_pos];
            }
        }
      else if (ifile.event() == InFile::END_SECTION)
        {
          if (section == "frame")
            {
              assert (audio_block);

              contents_pos++;
              audio_block = NULL;
            }

          assert (section != "");
          section = "";
        }
      else if (ifile.event() == InFile::INT)
        {
          if (section == "header")
            {
              if (ifile.event_name() == "zeropad")
                zeropad = ifile.event_int();
              else if (ifile.event_name() == "loop_start")
                loop_start = ifile.event_int();
              else if (ifile.event_name() == "loop_end")
                loop_end = ifile.event_int();
              else if (ifile.event_name() == "loop_type")
                loop_type = static_cast<LoopType> (ifile.event_int());
              else if (ifile.event_name() == "zero_values_at_start")
                zero_values_at_start = ifile.event_int();
              else if (ifile.event_name() == "sample_count")
                sample_count = ifile.event_int();
              else if (ifile.event_name() == "frame_count")
                {
                  int frame_count = ifile.event_int();

                  contents.clear();
                  contents.resize (frame_count);
                  contents_pos = 0;
                }
              else
                printf ("unhandled int %s %s\n", section.c_str(), ifile.event_name().c_str());
            }
          else
            assert (false);
        }
      else if (ifile.event() == InFile::FLOAT)
        {
          if (section == "header")
            {
              if (ifile.event_name() == "mix_freq")
                mix_freq = ifile.event_float();
              else if (ifile.event_name() == "frame_size_ms")
                frame_size_ms = ifile.event_float();
              else if (ifile.event_name() == "frame_step_ms")
                frame_step_ms = ifile.event_float();
              else if (ifile.event_name() == "attack_start_ms")
                attack_start_ms = ifile.event_float();
              else if (ifile.event_name() == "attack_end_ms")
                attack_end_ms = ifile.event_float();
              else if (ifile.event_name() == "start_ms")
                start_ms = ifile.event_float();
              else if (ifile.event_name() == "fundamental_freq")
                fundamental_freq = ifile.event_float();
              else
                printf ("unhandled float %s  %s\n", section.c_str(), ifile.event_name().c_str());
            }
          else
            assert (false);
        }
      else if (ifile.event() == InFile::FLOAT_BLOCK)
        {
          const vector<float>& fb = ifile.event_float_block();

          if (section == "header")
            {
              if (ifile.event_name() == "original_samples")
                {
                  original_samples = fb;
                }
              else
                printf ("unhandled float block %s  %s\n", section.c_str(), ifile.event_name().c_str());
            }
          else
            {
              assert (audio_block != NULL);
              if (ifile.event_name() == "noise")
                {
                  audio_block->noise = fb;
                }
              else if (ifile.event_name() == "freqs")
                {
                  audio_block->freqs = fb;

                  // ensure that freqs are sorted (we need that for LiveDecoder)
                  double old_freq = -1;

                  for (size_t f = 0; f < fb.size(); f++)
                    {
                      if (fb[f] <= old_freq)
                        {
                          printf ("frequency data is not sorted, can't play file\n");
                          return BSE_ERROR_PARSE_ERROR;
                        }
                      old_freq = fb[f];
                    }
                }
              else if (ifile.event_name() == "phases")
                {
                  audio_block->phases = fb;
                }
              else if (ifile.event_name() == "lpc_lsf_p")
                {
                  audio_block->lpc_lsf_p = fb;
                }
              else if (ifile.event_name() == "lpc_lsf_q")
                {
                  audio_block->lpc_lsf_q = fb;
                }
              else if (ifile.event_name() == "original_fft")
                {
                  audio_block->original_fft = fb;
                }
              else if (ifile.event_name() == "debug_samples")
                {
                  audio_block->debug_samples = fb;
                }
              else
                {
                  printf ("unhandled fblock %s %s\n", section.c_str(), ifile.event_name().c_str());
                  assert (false);
                }
            }
        }
      else if (ifile.event() == InFile::UINT16_BLOCK)
        {
          const vector<uint16_t>& ib = ifile.event_uint16_block();
          if (ifile.event_name() == "mags")
            {
              audio_block->mags = ib;
            }
          else
            {
              printf ("unhandled int16 block %s %s\n", section.c_str(), ifile.event_name().c_str());
              assert (false);
            }
        }
      else if (ifile.event() == InFile::READ_ERROR)
        {
          return BSE_ERROR_PARSE_ERROR;
        }
      else
        {
          return BSE_ERROR_PARSE_ERROR;
        }
      ifile.next_event();
    }
  return BSE_ERROR_NONE;
}


SpectMorph::Audio::Audio() :
  start_ms (0),
  zeropad (0),
  loop_type (LOOP_NONE), /* no loop */
  loop_start (-1),
  loop_end (-1)
{
  leak_debugger.add (this);
}

Audio::~Audio()
{
  leak_debugger.del (this);
}

/**
 * This function saves a SM-File.
 *
 * \param filename the name of the SM-File to be written
 * \returns a BseErrorType indicating saving loading was successful
 */
BseErrorType
SpectMorph::Audio::save (const string& filename) const
{
  GenericOut *out = StdioOut::open (filename);
  if (!out)
    {
      fprintf (stderr, "error: can't open output file '%s'.\n", filename.c_str());
      exit (1);
    }
  BseErrorType result = save (out);
  delete out; // close file

  return result;
}

BseErrorType
SpectMorph::Audio::save (GenericOut *file) const
{
  OutFile of (file, "SpectMorph::Audio", SPECTMORPH_BINARY_FILE_VERSION);
  assert (of.open_ok());

  of.begin_section ("header");
  of.write_float ("mix_freq", mix_freq);
  of.write_float ("frame_size_ms", frame_size_ms);
  of.write_float ("frame_step_ms", frame_step_ms);
  of.write_float ("attack_start_ms", attack_start_ms);
  of.write_float ("attack_end_ms", attack_end_ms);
  of.write_float ("start_ms", start_ms);
  of.write_float ("fundamental_freq", fundamental_freq);
  of.write_int ("zeropad", zeropad);
  of.write_int ("loop_type", loop_type);
  of.write_int ("loop_start", loop_start);
  of.write_int ("loop_end", loop_end);
  of.write_int ("zero_values_at_start", zero_values_at_start);
  of.write_int ("frame_count", contents.size());
  of.write_int ("sample_count", sample_count);
  of.write_float_block ("original_samples", original_samples);
  of.end_section();

  for (size_t i = 0; i < contents.size(); i++)
    {
      // ensure that freqs are sorted (we need that for LiveDecoder)
      double old_freq = -1;

      for (size_t f = 0; f < contents[i].freqs.size(); f++)
        {
          assert (contents[i].freqs[f] > old_freq);
          old_freq = contents[i].freqs[f];
        }

      of.begin_section ("frame");
      of.write_float_block ("noise", contents[i].noise);
      of.write_float_block ("freqs", contents[i].freqs);
      of.write_uint16_block ("mags", contents[i].mags);
      of.write_float_block ("phases", contents[i].phases);
      of.write_float_block ("lpc_lsf_p", contents[i].lpc_lsf_p);
      of.write_float_block ("lpc_lsf_q", contents[i].lpc_lsf_q);
      of.write_float_block ("original_fft", contents[i].original_fft);
      of.write_float_block ("debug_samples", contents[i].debug_samples);
      of.end_section();
    }
  return BSE_ERROR_NONE;
}

Audio *
Audio::clone() const
{
  // create a deep copy (by saving/loading)
  vector<unsigned char> audio_data;
  MemOut                audio_mo (&audio_data);

  save (&audio_mo);

  Audio *audio_clone = new Audio();
  GenericIn *in = MMapIn::open_mem (&audio_data[0], &audio_data[audio_data.size()]);
  audio_clone->load (in);
  delete in;

  return audio_clone;
}

bool
Audio::loop_type_to_string (LoopType loop_type, string& s)
{
  switch (loop_type)
    {
      case LOOP_NONE:
        {
          s = "loop-none";
          break;
        }
      case LOOP_FRAME_FORWARD:
        {
          s = "loop-frame-forward";
          break;
        }
      case LOOP_FRAME_PING_PONG:
        {
          s = "loop-frame-ping-pong";
          break;
        }
      case LOOP_TIME_FORWARD:
        {
          s = "loop-time-forward";
          break;
        }
      case LOOP_TIME_PING_PONG:
        {
          s = "loop-time-ping-pong";
          break;
        }
      default:
        {
          return false;  // unknown loop type
        }
    }
  return true;
}

bool
Audio::string_to_loop_type (const string& s, LoopType& loop_type)
{
  if (s == "loop-none")
    {
      loop_type = LOOP_NONE;
    }
  else if (s == "loop-frame-forward")
    {
      loop_type = LOOP_FRAME_FORWARD;
    }
  else if (s == "loop-frame-ping-pong")
    {
      loop_type = LOOP_FRAME_PING_PONG;
    }
  else if (s == "loop-time-forward")
    {
      loop_type = LOOP_TIME_FORWARD;
    }
  else if (s == "loop-time-ping-pong")
    {
      loop_type = LOOP_TIME_PING_PONG;
    }
  else
    {
      return false; // unknown loop type
    }
  return true;
}

namespace
{

struct PartialData
{
  float freq;
  float mag;
  float phase;
};

static bool
pd_cmp (const PartialData& p1, const PartialData& p2)
{
  return p1.freq < p2.freq;
}

}

void
AudioBlock::sort_freqs()
{
  // sort partials by frequency
  const size_t N = freqs.size();
  PartialData pvec[N];

  for (size_t p = 0; p < N; p++)
    {
      pvec[p].freq = freqs[p];
      pvec[p].mag = mags[p];
      pvec[p].phase = phases[p];
    }
  std::sort (pvec, pvec + N, pd_cmp);

  // replace partial data with sorted partial data
  for (size_t p = 0; p < N; p++)
    {
      freqs[p] = pvec[p].freq;
      mags[p] = pvec[p].mag;
      phases[p] = pvec[p].phase;
    }
}
