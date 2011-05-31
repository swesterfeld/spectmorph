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

#include "smlpc.hh"
#include "smmath.hh"
#include <bse/bsemathsignal.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <glib.h>
#include <stdio.h>

using namespace SpectMorph;
namespace ublas = boost::numeric::ublas;

using std::vector;
using std::complex;

static void
init_term_matrix (const std::vector<double>& lpc, const vector<float>& signal, vector< vector<double> >& term_matrix)
{
  term_matrix.clear();
  term_matrix.resize (lpc.size() + 1);
  for (size_t i = 0; i < term_matrix.size(); i++)
    term_matrix[i].resize (lpc.size() + 1);

  for (size_t i = lpc.size() + 1; i < signal.size(); i++)
    {
      for (size_t j = 0; j < lpc.size() + 1; j++)
        {
          double term_j;
          if (j != lpc.size())
            term_j = signal[i - 1 - j];
          else
            term_j = -signal[i];

          for (size_t k = 0; k < lpc.size() + 1; k++)
            {
              double term_k;
              if (k != lpc.size())
                term_k = signal[i - 1 - k];
              else
                term_k = -signal[i];

              term_matrix[j][k] += term_j * term_k;
            }
        }
    }
}

static void
solve (vector<double>& lpc, const vector< vector<double> >& term_matrix)
{
  // solve: first derivative of the prediction_error = 0 <=> optimal LPC coefficients in a

  ublas::permutation_matrix<double> P (lpc.size());
  ublas::matrix<double> A (lpc.size(), lpc.size());
  ublas::vector<double> x (lpc.size());
  for (size_t i = 0; i < lpc.size(); i++)
    {
      for (size_t j = 0; j < lpc.size(); j++)
        {
          A (i, j) = term_matrix[i][j];
        }
      x (i) = -term_matrix[lpc.size()][i];
    }
  ublas::lu_factorize (A, P);
  ublas::lu_substitute (A, P, x);
  for (size_t i = 0; i < lpc.size(); i++)
    lpc[i] = x[i];
}

void
LPC::compute_lpc (std::vector<double>& lpc, const float *begin, const float *end)
{
  vector<float> signal (begin, end);
  vector<vector<double> > term_matrix;

  init_term_matrix (lpc, signal, term_matrix);
  try {
    solve (lpc, term_matrix);
  } catch (...) {
  }
}

static void
find_lsf_roots (const vector<double>& p, vector<float>& roots)
{
  roots.clear();

  double a = 0, last_a = 1e5, last_last_a = 1e6; // decreasing to find root at 0 (which has increasing a)
  double delta_f = 0.0001;
  double last_f = 0;

  vector<double> sin_values (p.size());
  vector<double> cos_values (p.size());

  for (double f = 0; f < M_PI + 2 * delta_f; f += delta_f)
    {
      VectorSinParams vsparams;
      vsparams.mix_freq = 2 * M_PI;
      vsparams.freq = f;
      vsparams.phase = 0;
      vsparams.mag = 1;
      vsparams.mode = VectorSinParams::REPLACE;

      if (vsparams.freq < (delta_f / 2))  // fast_vector_sincos needs (vsparams.freq > 0)
        vsparams.freq += 2 * M_PI;

      fast_vector_sincos (vsparams, sin_values.begin(), sin_values.end(), cos_values.begin());

      complex<double> acc;
      for (size_t j = 0; j < p.size(); j++)
        {
          complex<double> z_j (cos_values[j], sin_values[j]);
          acc += p[j] * z_j;
        }

      a = abs (acc);
      if (last_a < a && last_a < last_last_a)
        roots.push_back (last_f);

      last_last_a = last_a;
      last_a = a;
      last_f = f;
    }
}

void
LPC::lpc2lsf (const std::vector<double>& lpc, std::vector<float>& lpc_lsf_p, std::vector<float>& lpc_lsf_q)
{
  vector<double> p, q;

  p.push_back (1);
  q.push_back (1);

  for (size_t i = 0; i < lpc.size(); i++)
    {
      p.push_back (-(lpc[i] + lpc[lpc.size() - 1 - i]));
      q.push_back (-(lpc[i] - lpc[lpc.size() - 1 - i]));
    }
  p.push_back (1);
  q.push_back (-1);

  find_lsf_roots (p, lpc_lsf_p);
  find_lsf_roots (q, lpc_lsf_q);
}

LPC::LSFEnvelope::LSFEnvelope()
{
  m_init = false;
}

bool
LPC::LSFEnvelope::init (const vector<float>& lpc_lsf_p, const vector<float>& lpc_lsf_q)
{
  m_init = false;
  g_return_val_if_fail (lpc_lsf_p.size() == lpc_lsf_q.size(), false);

  for (size_t j = 0; j < lpc_lsf_p.size(); j++)
    {
      complex<double> r_p (cos (lpc_lsf_p[j]), sin (lpc_lsf_p[j]));
      complex<double> r_q (cos (lpc_lsf_q[j]), sin (lpc_lsf_q[j]));

      if (j == lpc_lsf_p.size() - 1) // real root at nyquist
        p_real_root = r_p;
      else
        p_roots.push_back (r_p);

      if (j == 0)                  // real root at 0
        q_real_root = r_q;
      else
        q_roots.push_back (r_q);
    }
  m_init = true;

  return true;
}

double
LPC::LSFEnvelope::eval (double f)
{
  g_return_val_if_fail (m_init, 0);

  complex<double> z (cos (f), sin (f));
  complex<double> acc_p = 0.5;
  complex<double> acc_q = 0.5;

  acc_p *= (z - p_real_root);
  acc_q *= (z - q_real_root);

  for (size_t j = 0; j < p_roots.size(); j++)
    {
      complex<double> r_p (p_roots[j]);
      complex<double> r_q (q_roots[j]);

      acc_p *= (z - r_p) * (z - conj (r_p));
      acc_q *= (z - r_q) * (z - conj (r_q));
    }
  double value = 1 / abs (acc_p + acc_q);
  return value;
}

double
LPC::eval_lpc (const vector<double>& lpc, double f)
{
  complex<double> z (cos (f), sin (f));

  complex<double> acc = -1;
  for (int j = 0; j < int (lpc.size()); j++)
    acc += pow (z, -(j + 1)) * lpc[j];

  double value = 1 / abs (acc);
  return value;
}

static inline complex<double>
eval_z_complex (const vector<double>& lpc, complex<double> z)
{
  complex<double> acc = -1;
  complex<double> zinv = 1.0 / z;
  complex<double> zpow = zinv;
  for (size_t j = 0; j < lpc.size(); j++)
    {
      acc += zpow * lpc[j];
      zpow *= zinv;
    }
  return acc;
}

double
LPC::eval_z (const vector<double>& lpc, complex<double> z)
{
  return abs (eval_z_complex (lpc, z));
}

void
LPC::find_roots (const vector<double>& lpc, vector< complex<double> >& roots)
{
  roots.clear();
  while (roots.size() != lpc.size())
    {
      double root_re = g_random_double_range (-1, 1);
      double root_im = g_random_double_range (-1, 1);

      for (size_t i = 0; i < 100000; i++)
        {
          double value = eval_z (lpc, complex<double> (root_re, root_im));
          double delta_re, delta_im;
          if ((rand() & 3) == 0 || (value > 0.01))
            {
              double factor = bse_db_to_factor (g_random_double_range (-96, 0));
              delta_re = g_random_double_range (-0.1, 0.1) * factor;
              delta_im = g_random_double_range (-0.1, 0.1) * factor;
            }
          else
            {
              // Numerical derivative:
              // f'(z) ~= (f(z + epsilon) - f(z)) / epsilon
              const double epsilon = 1.0 / (1 << 30);
              complex<double> deriv = (eval_z_complex (lpc, complex<double> (root_re + epsilon, root_im)) -
                                       eval_z_complex (lpc, complex<double> (root_re, root_im))) / epsilon;

              // Newton step:
              // z_i+1 = z_i - f(z_i) / f'(z_i)
              complex<double> delta = -eval_z_complex (lpc, complex<double> (root_re, root_im)) / deriv;
              delta_re = delta.real();
              delta_im = delta.imag();
            }
          double new_value = eval_z (lpc, complex<double> (root_re + delta_re, root_im + delta_im));
          if (new_value < value)
            {
              root_re += delta_re;
              root_im += delta_im;
            }
          if (new_value < 1e-12)
            break;
        }
      complex<double> root_c (root_re, root_im);
      size_t t;
      for (t = 0; t < roots.size(); t++)
        {
          if (abs (root_c - roots[t]) < 0.01)
            break;
        }
      if (t == roots.size())
        {
          double value = eval_z (lpc, root_c);
          if (value < 1e-12)
            {
              roots.push_back (root_c);
            }
        }
    }
}
