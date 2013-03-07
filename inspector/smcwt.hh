// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

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
