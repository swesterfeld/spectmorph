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

#ifndef SPECTMORPH_CWT_HH
#define SPECTMORPH_CWT_HH

#include <vector>

#include <QObject>

#include "smcommon.hh"

namespace SpectMorph {

class FFTThread;

class CWT : public QObject
{
  Q_OBJECT
public:
  std::vector< std::vector<float> > analyze (const std::vector<float>& signal, const AnalysisParams& params, FFTThread *fft_thread = 0);
  std::vector< std::vector<float> > analyze_slow (const std::vector<float>& signal, FFTThread *fft_thread = 0);
  void make_png (std::vector< std::vector<float> >& results);

signals:
  void signal_progress (double progress);
};

}

#endif
