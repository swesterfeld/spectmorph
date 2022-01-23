/*****************************************************************************

        Upsampler2xSse.hpp
        Author: Laurent de Soras, 2020

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.

*Tab=3***********************************************************************/



#if ! defined (hiir_Upsampler2xSse_CODEHEADER_INCLUDED)
#define hiir_Upsampler2xSse_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "hiir/StageProcSseV2.h"

#include <xmmintrin.h>

#include <cassert>



namespace hiir
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*
==============================================================================
Name: ctor
Throws: Nothing
==============================================================================
*/

template <int NC>
Upsampler2xSse <NC>::Upsampler2xSse ()
:	_filter ()
{
	for (int i = 0; i < _nbr_stages + 1; ++i)
	{
		_mm_store_ps (_filter [i]._coef, _mm_setzero_ps ());
	}
	if (NBR_COEFS < _nbr_stages * 2)
	{
		_filter [_nbr_stages]._coef [0] = 1;
	}

	clear_buffers ();
}



/*
==============================================================================
Name: set_coefs
Description:
	Sets filter coefficients. Generate them with the PolyphaseIir2Designer
	class.
	Call this function before doing any processing.
Input parameters:
	- coef_arr: Array of coefficients. There should be as many coefficients as
		mentioned in the class template parameter.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Upsampler2xSse <NC>::set_coefs (const double coef_arr [NBR_COEFS])
{
	assert (coef_arr != nullptr);

	for (int i = 0; i < NBR_COEFS; ++i)
	{
		const int      stage = (i / _stage_width) + 1;
		const int      pos   = (i ^ 1) & (_stage_width - 1);
		_filter [stage]._coef [pos] = DataType (coef_arr [i]);
	}
}



/*
==============================================================================
Name: process_sample
Description:
	Upsamples (x2) the input sample, generating two output samples.
Input parameters:
	- input: The input sample.
Output parameters:
	- out_0: First output sample.
	- out_1: Second output sample.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Upsampler2xSse <NC>::process_sample (float &out_0, float &out_1, float input)
{
	auto           x = _mm_set1_ps (input);
	StageProcSseV2 <_nbr_stages>::process_sample_pos (x, &_filter [0]);
	out_1 = _mm_cvtss_f32 (x);
	x = _mm_shuffle_ps (x, x, 1);
	out_0 = _mm_cvtss_f32 (x);
}



/*
==============================================================================
Name: process_block
Description:
	Upsamples (x2) the input sample block.
	Input and output blocks may overlap, see assert() for details.
Input parameters:
	- in_ptr: Input array, containing nbr_spl samples.
	- nbr_spl: Number of input samples to process, > 0
Output parameters:
	- out_0_ptr: Output sample array, capacity: nbr_spl * 2 samples.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Upsampler2xSse <NC>::process_block (float out_ptr [], const float in_ptr [], long nbr_spl)
{
	assert (out_ptr != nullptr);
	assert (in_ptr  != nullptr);
	assert (out_ptr >= in_ptr + nbr_spl || in_ptr >= out_ptr + nbr_spl);
	assert (nbr_spl > 0);

	for (long pos = 0; pos < nbr_spl; ++pos)
	{
#if 0
		process_sample (out_ptr [pos * 2], out_ptr [pos * 2 + 1], in_ptr [pos]);
#else
	auto           x = _mm_set1_ps (in_ptr [pos]);
	StageProcSseV2 <_nbr_stages>::process_sample_pos (x, &_filter [0]);
	x = _mm_shuffle_ps (x, x, 1);
	_mm_storel_pi (reinterpret_cast <__m64 *> (out_ptr + pos * 2), x);
#endif
	}
}



/*
==============================================================================
Name: clear_buffers
Description:
	Clears filter memory, as if it processed silence since an infinite amount
	of time.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Upsampler2xSse <NC>::clear_buffers ()
{
	for (int i = 0; i < _nbr_stages + 1; ++i)
	{
		_mm_store_ps (_filter [i]._mem, _mm_setzero_ps ());
	}
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}  // namespace hiir



#endif   // hiir_Upsampler2xSse_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
