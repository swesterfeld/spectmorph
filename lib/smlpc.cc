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

  p_a.resize (lpc_lsf_p.size() - 1);
  p_b.resize (lpc_lsf_p.size() - 1);
  q_a.resize (lpc_lsf_p.size() - 1);
  q_b.resize (lpc_lsf_p.size() - 1);

  for (size_t j = 0; j < lpc_lsf_p.size(); j++)
    {
      complex<double> r_p (cos (lpc_lsf_p[j]), sin (lpc_lsf_p[j]));
      complex<double> r_q (cos (lpc_lsf_q[j]), sin (lpc_lsf_q[j]));

      if (j == lpc_lsf_p.size() - 1) // real root at nyquist
        p_real_root = r_p.real();
      else
        {
          p_a[j] = -2 * r_p.real();
          p_b[j] = r_p.real() * r_p.real() + r_p.imag() * r_p.imag();
        }

      if (j == 0)                  // real root at 0
        q_real_root = r_q.real();
      else
        {
          q_a[j - 1] = -2 * r_q.real();
          q_b[j - 1] = r_q.real() * r_q.real() + r_q.imag() * r_q.imag();
        }
    }
  m_init = true;

  return true;
}

static inline complex<double>
xmul (complex<double> a, complex<double> b)
{
  const double rr = a.real() * b.real();
  const double ri = a.real() * b.imag();
  const double ir = a.imag() * b.real();
  const double ii = a.imag() * b.imag();

  return complex<double> (rr - ii, ir + ri);
}

static inline double
xabs (complex<double> z)
{
  const double rr = z.real() * z.real();
  const double ii = z.imag() * z.imag();

  return sqrt (rr + ii);
}

double
LPC::LSFEnvelope::eval (double f)
{
  g_return_val_if_fail (m_init, 0);

  complex<double> z (cos (f), sin (f));
  complex<double> z2 = xmul (z, z);
  complex<double> acc_p = 0.5;
  complex<double> acc_q = 0.5;

  acc_p = xmul (acc_p, z - p_real_root);
  acc_q = xmul (acc_q, z - q_real_root);

  for (size_t j = 0; j < p_a.size(); j++)
    {
      acc_p = xmul (acc_p, z2 + z * p_a[j] + p_b[j]);
      acc_q = xmul (acc_q, z2 + z * q_a[j] + q_b[j]);
    }
  double value = 1 / xabs (acc_p + acc_q);
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

static inline complex<long double>
eval_z_complex (const vector< complex<long double> >& lpc, complex<long double> z, long double& err)
{
  complex<long double> acc = lpc.back();
  complex<long double> zinv = 1.0L / z;
  long double          abs_zinv = abs (zinv);

  /* err is the estimated error of this polynomial evaluation, from
   * Numerical Recipies, Third Edition, Section 9.5.3
   */
  err = abs (acc);

  for (int j = lpc.size() - 2; j >= 0; j--)
    {
      acc = lpc[j] + zinv * acc;
      err = abs (acc) + abs_zinv * err;
    }
  err *= 1e-10;
  return acc;
}

long double
LPC::eval_z (const vector<double>& lpc_real, complex<long double> z)
{
  // convert real coefficients to complex coefficients
  // FIXME: constant coefficient
  vector< complex<long double> > lpc (lpc_real.begin(), lpc_real.end());
  long double err;
  return abs (eval_z_complex (lpc, z, err));
}

// first coefficient => z^n
// last coefficient  => constant
template<class Real, class Complex> static inline Complex
eval_poly_dk (const vector<Real>& lpc, Complex z)
{
  Complex acc = lpc.front();

  for (size_t j = 1; j < lpc.size(); j++)
    acc = lpc[j] + z * acc;
  return acc;
}

// find roots using Durand-Kerner method
bool
LPC::find_roots (const vector<double>& lpc, vector< complex<double> >& roots_out)
{
  vector< complex<long double> > roots (lpc.size());

  /* For LPC, our polynomial is -1 + a(0) * z^-1 + a(1) * z^-2 + ... + a(n-1) * z^-n
   * to find roots, we multiply it with -z^n, to get z^n + a(0) * z^(n-1) + a(1) * z^(n-2) ... + a(n-1)
   *
   * This means:
   *   - leading coefficient is 1   (as required by Durand-Kerner algorithm)
   *   - roots remain the same
   */
  vector<long double> poly (lpc.size() + 1);
  poly[0] = 1;
  for (size_t i = 0; i < lpc.size(); i++)
    poly[i + 1] = -lpc[i];

  bool restart;
  do
    {
      restart = false;

      // start with random roots
      for (size_t i = 0; i < roots.size(); i++)
        roots[i] = complex<long double> (g_random_double_range (-1, 1),
                                         g_random_double_range (-1, 1));

      for (size_t x = 0; x < 100; x++)
        {
          for (size_t i = 0; i < roots.size(); i++)
            {
              complex<long double> value = eval_poly_dk (poly, roots[i]);

              for (size_t j = 0; j < roots.size(); j++)
                {
                  if (j != i)
                    value /= (roots[i] - roots[j]);
                }
              roots[i] -= value;
            }
        }
      roots_out.resize (lpc.size());
      for (size_t i = 0; i < roots.size(); i++)
        {
          complex<long double> root = roots[i];
          if (!finite (root.real()) || !finite (root.imag()))
            restart = true;
        }
    }
  while (restart);

  // convert "long double" precision roots down to "double" precision roots
  roots_out.resize (roots.size());
  std::copy (roots.begin(), roots.end(), roots_out.begin());

  return true;
}

/* multiply coefficient array to add one root to the polynomial */
static void
inflate (vector< complex<long double> >& lpc, complex<long double> root)
{
  //root = 1.0L / root;
  vector< complex<long double> > new_lpc (lpc.size() + 1);
  for (size_t i = 0; i < lpc.size(); i++)
    {
      new_lpc[i + 1] = lpc[i];
      new_lpc[i] -= root * lpc[i];
    }
  lpc = new_lpc;
}

void
LPC::roots2lpc (const vector< complex<double> >& roots, vector<double>& lpc)
{
  vector<complex<long double> > new_lpc;
  new_lpc.push_back (-1);
  for (size_t i = 0; i < roots.size(); i++)
    inflate (new_lpc, roots[i]);
  lpc.resize (roots.size());
  for (size_t i = 0; i < roots.size(); i++)
    lpc[lpc.size() - 1 - i] = new_lpc[i].real();
}

void
LPC::make_stable_roots (vector< complex<double> >& roots)
{
  double max_abs = 1 - 1e-4;  // maximum abs value for roots (to prevent problems in LPC->LSF conversion)
  for (size_t i = 0; i < roots.size(); i++)
    {
      // move root into unit circle if necessary
      if (abs (roots[i]) > 1.0)
        roots[i] = 1.0 / conj (roots[i]);
      // ensure that |roots[i]| <= max_abs;
      if (abs (roots[i]) > max_abs)
        roots[i] = (roots[i] / abs (roots[i])) * max_abs;
    }
}

void
LPC::lsf2lpc (const vector<float>& lsf_p, const vector<float>& lsf_q, vector<double>& lpc)
{
  vector< complex<long double> > p, q;
  p.push_back (1);
  q.push_back (1);
  for (size_t i = 0; i < lsf_p.size(); i++)
    {
      complex<double> r_p (cos (lsf_p[i]), sin (lsf_p[i]));
      complex<double> r_q (cos (lsf_q[i]), sin (lsf_q[i]));
      if (i == lsf_p.size() - 1) // real root at nyquist
        {
          inflate (p, r_p);
        }
      else
        {
          inflate (p, r_p);
          inflate (p, conj (r_p));
        }
      if (i == 0)                  // real root at 0
        {
          inflate (q, r_q);
        }
      else
        {
          inflate (q, r_q);
          inflate (q, conj (r_q));
        }
    }
  lpc.clear();
  for (size_t i = p.size() - 2; i > 0; i--)
    {
      complex<long double> value = (p[i] + q[i]);
      lpc.push_back (-0.5 * value.real());
    }
}
