// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#include "smfftthread.hh"
#include "smtimefreqview.hh"
#include "smfft.hh"
#include "smcwt.hh"
#include "smlpc.hh"

#include <bse/bseblockutils.hh>
#include <bse/bseglobals.h>
#include <bse/bsemathsignal.h>

#include <QSocketNotifier>

#include <sys/poll.h>
#include <stdio.h>
#include <assert.h>

using namespace SpectMorph;

using std::vector;
using std::max;

void
FFTThread::run()
{
  struct pollfd poll_fds[1];
  poll_fds[0].fd = fft_thread_wakeup_pfds[0];
  poll_fds[0].events = POLLIN;
  poll_fds[0].revents = 0;

  while (1)
    {
      if (poll (poll_fds, 1, -1) > 0)
        {
          char c;
          read (fft_thread_wakeup_pfds[0], &c, 1);
        }

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

          if (commands.empty())       // check if user aborted this command
            command_results.push_back (c);
          else
            delete c;

          // wakeup main thread
          while (write (main_thread_wakeup_pfds[1], "W", 1) != 1)
            ;
        }
      command_mutex.unlock();
    }
}

void
FFTThread::set_command_progress (double progress)
{
  QMutexLocker lock (&command_mutex);
  command_progress = progress;

  // wakeup main thread
  while (write (main_thread_wakeup_pfds[1], "W", 1) != 1)
    ;
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
  pipe (fft_thread_wakeup_pfds);
  pipe (main_thread_wakeup_pfds);
  pthread_create (&thread, NULL, thread_start, this);

  QSocketNotifier *socket_notifier = new QSocketNotifier (main_thread_wakeup_pfds[0], QSocketNotifier::Read, this);
  connect (socket_notifier, SIGNAL (activated (int)), this, SLOT (on_result_available()));
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
  return bse_db_from_factor (value, -200);
}

AnalysisCommand::AnalysisCommand (GslDataHandle *dhandle, const AnalysisParams& analysis_params, FFTThread *fft_thread) :
  fft_thread (fft_thread),
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
AnalysisCommand::set_progress (double progress)
{
  fft_thread->set_command_progress (progress);
}

void
AnalysisCommand::execute_cwt()
{
  CWT cwt;

  vector<float> signal;
  vector<float> block (1024);

  const uint64 n_values = gsl_data_handle_length (dhandle);
  uint64 pos = 0;
  while (pos < n_values)
    {
      /* read data from file */
      uint64 r = gsl_data_handle_read (dhandle, pos, block.size(), &block[0]);

      for (uint64 t = 0; t < r; t++)
        signal.push_back (block[t]);
      pos += r;
    }

  double mix_freq = gsl_data_handle_mix_freq (dhandle);

  connect (&cwt, SIGNAL (signal_progress (double)), this, SLOT (set_progress (double)));

  vector< vector<float> > results;
  results = cwt.analyze (signal, analysis_params, fft_thread);

  size_t width = 0;
  size_t height = results.size();
  if (!results.empty())
    width = results[0].size();

  image.resize (width, height);

  float max_value = -200;
  for (vector< vector<float> >::iterator fi = results.begin(); fi != results.end(); fi++)
    {
      for (vector<float>::iterator mi = fi->begin(); mi != fi->end(); mi++)
        {
          *mi = value_scale (*mi);
          max_value = max (max_value, *mi);
        }
    }
  int    *p = image.get_pixels();
  size_t  row_stride = image.get_rowstride();
  for (size_t y = 0; y < height; y++)
    {
      for (size_t x = 0; x < results[y].size(); x++)
        {
          const size_t src_y = (height - 1 - y);
          p[x] = (results[src_y][x] - max_value) * 256;  // 8 bits fixed point
        }
      p += row_stride;
    }
}

void
AnalysisCommand::execute_lpc()
{
  vector<float> signal;
  vector<float> block (1024);

  const uint64 n_values = gsl_data_handle_length (dhandle);
  uint64 pos = 0;
  while (pos < n_values)
    {
      /* read data from file */
      uint64 r = gsl_data_handle_read (dhandle, pos, block.size(), &block[0]);

      for (uint64 t = 0; t < r; t++)
        signal.push_back (block[t]);
      pos += r;
    }

  double mix_freq = gsl_data_handle_mix_freq (dhandle);

  vector< vector<float> > results;

  for (size_t start = 0; start < n_values; start += 1500)
    {
      size_t end = start + mix_freq * 0.030; // 30 ms;
      if (start < end && end < signal.size())
        {
          vector<double> lpc (50);
          LPC::compute_lpc (lpc, &signal[start], &signal[end]);
          vector<float> result;
          for (float freq = 0; freq < M_PI; freq += 0.001)
            {
              float mag = LPC::eval_lpc (lpc, freq);
              result.push_back (mag);
            }
          results.push_back (result);
        }
      set_progress (CLAMP (start / double (n_values), 0.0, 1.0));
    }

  size_t width = results.size();
  size_t height = 0;
  if (!results.empty())
    height = results[0].size();

  image.resize (width, height);

  float max_value = -200;
  for (vector< vector<float> >::iterator fi = results.begin(); fi != results.end(); fi++)
    {
      for (vector<float>::iterator mi = fi->begin(); mi != fi->end(); mi++)
        {
          *mi = value_scale (*mi);
          max_value = max (max_value, *mi);
        }
    }
  int    *p = image.get_pixels();
  size_t  row_stride = image.get_rowstride();
  for (size_t frame = 0; frame < width; frame++)
    {
      for (size_t y = 0; y < height; y++)
        {
          size_t src_y = height - y - 1;
          p[src_y * row_stride] = (results[frame][y] - max_value) * 256;  // 8 bits fixed point
        }
      p++;
    }
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

  if (analysis_params.transform_type == SM_TRANSFORM_CWT)
    {
      execute_cwt();
      return;
    }
  else if (analysis_params.transform_type == SM_TRANSFORM_LPC)
    {
      execute_lpc();
      return;
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

          result.mags.push_back (value_scale (sqrt (re * re + im * im)));
        }
      results.push_back (result);

      set_progress (CLAMP (pos_ms / len_ms, 0.0, 1.0));

      if (fft_thread->command_is_obsolete())      // abort analysis if user requested a new one
        break;
    }

  FFT::free_array_float (fft_in);
  FFT::free_array_float (fft_out);

  size_t height = 0;
  if (!results.empty())
    height = results[0].mags.size();

  image.resize (results.size(), height);
  float max_value = 0;
  for (vector<FFTResult>::const_iterator fi = results.begin(); fi != results.end(); fi++)
    {
      for (vector<float>::const_iterator mi = fi->mags.begin(); mi != fi->mags.end(); mi++)
        {
          max_value = max (max_value, *mi);
        }
    }
  int    *p = image.get_pixels();
  size_t  row_stride = image.get_rowstride();
  for (size_t frame = 0; frame < results.size(); frame++)
    {
      for (size_t m = 0; m < results[frame].mags.size(); m++)
        {
          int y = results[frame].mags.size() - 1 - m;
          p[row_stride * y] = (results[frame].mags[m] - max_value) * 256;  // 8 bits fixed point
        }
      p++;
    }
}

void
FFTThread::compute_image (PixelArray& image, GslDataHandle *dhandle, const AnalysisParams& params)
{
  QMutexLocker lock (&command_mutex);
  commands.push_back (new AnalysisCommand (dhandle, params, this));

  // wakeup FFT thread
  while (write (fft_thread_wakeup_pfds[1], "W", 1) != 1)
    ;
}

bool
FFTThread::get_result (PixelArray& image)
{
  QMutexLocker lock (&command_mutex);

  // clear wakeup pipe
  struct pollfd poll_fds[1];
  poll_fds[0].fd = main_thread_wakeup_pfds[0];
  poll_fds[0].events = POLLIN;
  poll_fds[0].revents = 0;

  if (poll (poll_fds, 1, 0) > 0)
    {
      char c;
      read (main_thread_wakeup_pfds[0], &c, 1);
    }

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

void
FFTThread::on_result_available()
{
  Q_EMIT result_available();
}

double
FFTThread::get_progress()
{
  QMutexLocker lock (&command_mutex);
  return command_progress;
}

bool
FFTThread::command_is_obsolete()
{
  QMutexLocker lock (&command_mutex);
  return !commands.empty();
}
