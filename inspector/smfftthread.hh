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


#ifndef SPECTMORPH_FFTTHREAD_HH
#define SPECTMORPH_FFTTHREAD_HH

#include <birnet/birnet.hh>
#include <birnet/birnetthread.hh>
#include <bse/bseloader.h>

#include "smcommon.hh"
#include "smpixelarray.hh"

namespace SpectMorph
{

class FFTThread
{
public:
  struct Command {
    virtual void execute() = 0;
  };

protected:
  Birnet::Mutex           command_mutex;
  std::vector<Command *>  commands;
  std::vector<Command *>  command_results;

  int                     fft_thread_wakeup_pfds[2];
  int                     main_thread_wakeup_pfds[2];

  pthread_t thread;

  bool on_result_available (Glib::IOCondition io_condition);

public:
  FFTThread();
  ~FFTThread();

  void run();
  void compute_image (PixelArray& image, GslDataHandle *dhandle, AnalysisParams& params);
  bool get_result (PixelArray& image);

  static FFTThread *the();

  sigc::signal<void> signal_result_available;
};

}

#endif
