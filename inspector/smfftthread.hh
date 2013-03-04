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

#include <QObject>

namespace SpectMorph
{

class FFTThread : public QObject
{
  Q_OBJECT
public:
  struct Command : public QObject {
    virtual void execute() = 0;
  };

protected:
  Birnet::Mutex           command_mutex;
  std::vector<Command *>  commands;
  std::vector<Command *>  command_results;
  double                  command_progress;

  int                     fft_thread_wakeup_pfds[2];
  int                     main_thread_wakeup_pfds[2];

  pthread_t thread;

public:
  FFTThread();
  ~FFTThread();

  void set_command_progress (double progress);
  bool command_is_obsolete();

  void run();
  void compute_image (PixelArray& image, GslDataHandle *dhandle, const AnalysisParams& params);
  bool get_result (PixelArray& image);
  double get_progress();

  static FFTThread *the();

public slots:
  void on_result_available();

signals:
  void result_available();
};

class AnalysisCommand : public FFTThread::Command
{
  Q_OBJECT
public:
  FFTThread              *fft_thread;
  GslDataHandle          *dhandle;
  std::vector<FFTResult>  results;
  PixelArray              image;
  AnalysisParams          analysis_params;

  AnalysisCommand (GslDataHandle *dhandle, const AnalysisParams& analysis_params, FFTThread *fft_thread);
  ~AnalysisCommand();
  void execute();
  void execute_cwt();
  void execute_lpc();

public slots:
  void set_progress (double progress);
};

}

#endif
