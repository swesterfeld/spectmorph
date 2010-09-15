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


#ifndef SPECTMORPH_FFT_HH
#define SPECTMORPH_FFT_HH

#include <sys/types.h>

namespace SpectMorph
{

namespace FFT
{

float *new_array_float (size_t N);
void   free_array_float (float *f);

void   fftar_float (size_t N, float *in, float *out);
void   fftsr_float (size_t N, float *in, float *out);
void   fftac_float (size_t N, float *in, float *out);
void   fftsc_float (size_t N, float *in, float *out);

void   use_gsl_fft (bool enabled);

void   save_wisdom();
void   load_wisdom();

}

}

#endif
