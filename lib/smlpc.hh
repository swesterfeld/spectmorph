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

double eval_lpc (const std::vector<double>& lpc, double f);

class LSFEnvelope {
  std::vector< std::complex<double> > p_roots;
  std::complex<double>                p_real_root;

  std::vector< std::complex<double> > q_roots;
  std::complex<double>                q_real_root;

  bool                                m_init;
public:
  LSFEnvelope();

  bool init (const std::vector<float>& lpc_lsf_p, const std::vector<float>& lpc_lsf_q);
  double eval (double f);
};

long double eval_z (const std::vector<double>& lpc, std::complex<long double> z);
void find_roots (const std::vector<double>& lpc, std::vector< std::complex<double> >& roots);
void roots2lpc (const std::vector< std::complex<double> >& roots, std::vector<double>& lpc);
}

}

#endif
