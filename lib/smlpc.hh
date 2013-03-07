// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LPC_HH
#define SPECTMORPH_LPC_HH

#include <vector>
#include <complex>

namespace SpectMorph
{

namespace LPC
{

void compute_lpc (std::vector<double>& lpc, const float *begin, const float *end);
void lpc2lsf (const std::vector<double>& lpc, std::vector<float>& lpc_lsf_p, std::vector<float>& lpc_lsf_q);
void lsf2lpc (const std::vector<float>& lsf_p, const std::vector<float>& lsf_q, std::vector<double>& lpc);

double eval_lpc (const std::vector<double>& lpc, double f);

class LSFEnvelope {
  std::vector<double>                 p_a;
  std::vector<double>                 p_b;
  double                              p_real_root;

  std::vector<double>                 q_a;
  std::vector<double>                 q_b;
  double                              q_real_root;

  bool                                m_init;
public:
  LSFEnvelope();

  bool init (const std::vector<float>& lpc_lsf_p, const std::vector<float>& lpc_lsf_q);
  double eval (double f);
};

long double eval_z (const std::vector<double>& lpc, std::complex<long double> z);
bool find_roots (const std::vector<double>& lpc, std::vector< std::complex<double> >& roots);
void roots2lpc (const std::vector< std::complex<double> >& roots, std::vector<double>& lpc);
void make_stable_roots (std::vector< std::complex<double> >& roots);

}

}

#endif
