/*
 * Copyright (C) 2010 Stefan Westerfeld
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

#include "smfftthread.hh"
#include "smtimefreqview.hh"
#include "smfft.hh"

#include <bse/bseblockutils.hh>
#include <bse/bseglobals.h>
#include <bse/bsemathsignal.h>
#include <stdio.h>
#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::max;

void
FFTThread::run()
{
  while (1)
    {
      command_mutex.lock();
      /* we currently hard code that newer commands are always executed
       * instead of older commands, so that only the newest command (and
       * command result) is important
       */
      if (!commands.empty())
        {
          Command *c = commands.back();
          commands.pop_back();

          // delete commands we're not going to execute
          for (vector<Command *>::iterator ci = commands.begin(); ci != commands.end(); ci++)
            delete (*ci);
          commands.clear();

          command_mutex.unlock();
          c->execute();
          command_mutex.lock();

          // throw away old results (obsolete anyway)
          for (vector<Command *>::iterator ri = command_results.begin(); ri != command_results.end(); ri++)
            delete (*ri);
          command_results.clear();

          command_results.push_back (c);
        }
      command_mutex.unlock();
      usleep (100);
    }
}

void*
thread_start (void *arg)
{
  FFTThread *instance = static_cast<FFTThread *> (arg);
  instance->run();
  return NULL;
}

static FFTThread *the_instance = NULL;

FFTThread::FFTThread()
{
  assert (the_instance == NULL);
  the_instance = this;
  pthread_create (&thread, NULL, thread_start, this);
}

FFTThread::~FFTThread()
{
  assert (the_instance != NULL);
  the_instance = NULL;
}

FFTThread *
FFTThread::the()
{
  return the_instance;
}

static float
value_scale (float value)
{
  if (true)
    {
      double db = bse_db_from_factor (value, -200);
      if (db > -90)
        return db + 90;
      else
        return 0;
    }
  else
    return value;
}

struct AnalysisCommand : public FFTThread::Command
{
  GslDataHandle    *dhandle;
  vector<FFTResult> results;
  PixelArray        image;
  AnalysisParams    analysis_params;

  AnalysisCommand (GslDataHandle *dhandle, AnalysisParams& analysis_params);
  ~AnalysisCommand();
  void execute();
};

AnalysisCommand::AnalysisCommand (GslDataHandle *dhandle, AnalysisParams& analysis_params) :
  dhandle (dhandle),
  analysis_params (analysis_params)
{
  gsl_data_handle_ref (dhandle);
}

AnalysisCommand::~AnalysisCommand()
{
  gsl_data_handle_unref (dhandle);
}

void
AnalysisCommand::execute()
{
  BseErrorType error = gsl_data_handle_open (dhandle);
  if (error)
    {
      fprintf (stderr, "FFTThread: can't open the input data handle: %s\n", bse_error_blurb (error));
      exit (1);
    }

  if (gsl_data_handle_n_channels (dhandle) != 1)
    {
      fprintf (stderr, "Currently, only mono files are supported.\n");
      exit (1);
    }

  size_t frame_size = analysis_params.frame_size_ms * gsl_data_handle_mix_freq (dhandle) / 1000.0;
  size_t block_size = 1;
  while (block_size < frame_size)
    block_size *= 2;

  vector<float> block (block_size);
  vector<float> window (block_size);

  size_t zeropad = 4;
  size_t fft_size = block_size * zeropad;

  float *fft_in = FFT::new_array_float (fft_size);
  float *fft_out = FFT::new_array_float (fft_size);

  for (guint i = 0; i < window.size(); i++)
    {
      if (i < frame_size)
        window[i] = bse_window_cos (2.0 * i / frame_size - 1.0);
      else
        window[i] = 0;
    }


  double len_ms = gsl_data_handle_length (dhandle) * 1000.0 / gsl_data_handle_mix_freq (dhandle);
  for (double pos_ms = analysis_params.frame_step_ms * 0.5 - analysis_params.frame_size_ms; pos_ms < len_ms; pos_ms += analysis_params.frame_step_ms)
    {
      int64 pos = pos_ms / 1000.0 * gsl_data_handle_mix_freq (dhandle);
      int64 todo = block.size(), offset = 0;
      const int64 n_values = gsl_data_handle_length (dhandle);

      while (todo)
        {
          int64 r = 0;
          if ((pos + offset) < 0)
            {
              r = 1;
              block[offset] = 0;
            }
          else if (pos + offset < n_values)
            {
              r = gsl_data_handle_read (dhandle, pos + offset, todo, &block[offset]);
            }
          if (r > 0)
            {
              offset += r;
              todo -= r;
            }
          else  // last block
            {
              while (todo)
                {
                  block[offset++] = 0;
                  todo--;
                }
            }
        }
      Bse::Block::mul (block_size, &block[0], &window[0]);
      for (size_t i = 0; i < fft_size; i++)
        {
          if (i < block_size)
            fft_in[i] = block[i];
          else
            fft_in[i] = 0;
        }
      FFT::fftar_float (fft_size, fft_in, fft_out);
      FFTResult result;
      fft_out[1] = 0; // special packing
      for (size_t i = 0; i < fft_size; i += 2)
        {
          double re = fft_out[i];
          double im = fft_out[i + 1];

          result.mags.push_back (sqrt (re * re + im * im));
        }
      results.push_back (result);
    }
  size_t height = 0;
  if (!results.empty())
    height = results[0].mags.size();

  image.resize (results.size(), height);

  float max_value = 0;
  for (vector<FFTResult>::const_iterator fi = results.begin(); fi != results.end(); fi++)
    {
      for (vector<float>::const_iterator mi = fi->mags.begin(); mi != fi->mags.end(); mi++)
        {
          max_value = max (max_value, value_scale (*mi));
        }
    }
  guchar *p = image.get_pixels();
  size_t  row_stride = image.get_rowstride();
  for (size_t frame = 0; frame < results.size(); frame++)
    {
      for (size_t m = 0; m < results[frame].mags.size(); m++)
        {
          double value = value_scale (results[frame].mags[m]);
          value /= max_value;
          int y = results[frame].mags.size() - 1 - m;
          p[row_stride * y] = value * 255;
        }
      p++;
    }
}

void
FFTThread::compute_image (PixelArray& image, GslDataHandle *dhandle, AnalysisParams& params)
{
  Birnet::AutoLocker lock (command_mutex);
  commands.push_back (new AnalysisCommand (dhandle, params));
}

bool
FFTThread::get_result (PixelArray& image)
{
  Birnet::AutoLocker lock (command_mutex);

  if (!command_results.empty())
    {
      AnalysisCommand *ac = dynamic_cast<AnalysisCommand *> (command_results[0]);
      image = ac->image;
      delete ac;
      command_results.erase (command_results.begin());

      return true;
    }
  return false;
}
