// Licensed GNU LGPL v3 or later: http://www.gnu.org/licenses/lgpl.html

#ifndef SPECTMORPH_LPC_ZTRANS_HH
#define SPECTMORPH_LPC_ZTRANS_HH

#include <complex>
#include <vector>
#include "smlpc.hh"

namespace SpectMorph
{

struct LPCZFunction
{
  virtual double eval (std::complex<double> z) const = 0;
};

struct LPCZFunctionLPC : public LPCZFunction
{
  std::vector<double> a;

  LPCZFunctionLPC (const std::vector<double>& a)
    : a (a)
  {
  }
  double
  eval (std::complex<double> z) const
  {
    return LPC::eval_z (a, z);
  }
};


#if 0
GdkPixbuf *lpc_z_transform (const LPCZFunction& zfunc, const std::vector< std::complex<double> >& roots);
GdkPixbuf *lpc_z_transform (const std::vector<double>& a, const std::vector< std::complex<double> >& roots);
#endif

}

#endif
