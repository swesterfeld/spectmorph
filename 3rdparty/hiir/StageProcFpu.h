/*****************************************************************************

        StageProcFpu.h
        Author: Laurent de Soras, 2005

Template parameters:

- REMAINING: Number of remaining coefficients to process, >= 0

- DT: Data type (float or double)



--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (hiir_StageProc_HEADER_INCLUDED)
#define hiir_StageProc_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "hiir/def.h"
#include "hiir/StageDataFpu.h"



namespace hiir
{



template <int REMAINING, typename DT>
class StageProcFpu
{

	static_assert ((REMAINING >= 0), "REMAINING must be >= 0");

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	static hiir_FORCEINLINE void
	               process_sample_pos (const int nbr_coefs, DT &spl_0, DT &spl_1, StageDataFpu <DT> *stage_arr);
	static hiir_FORCEINLINE void
	               process_sample_neg (const int nbr_coefs, DT &spl_0, DT &spl_1, StageDataFpu <DT> *stage_arr);



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	               StageProcFpu ()                                          = delete;
	               StageProcFpu (const StageProcFpu <REMAINING, DT> &other) = delete;
	               StageProcFpu (StageProcFpu <REMAINING, DT> &&other)      = delete;
	               ~StageProcFpu ()                                         = delete;
	StageProcFpu <REMAINING, DT> &
	               operator = (const StageProcFpu <REMAINING, DT> &other)   = delete;
	StageProcFpu <REMAINING, DT> &
	               operator = (StageProcFpu <REMAINING, DT> &&other)        = delete;
	bool           operator == (const StageProcFpu <REMAINING, DT> &other)  = delete;
	bool           operator != (const StageProcFpu <REMAINING, DT> &other)  = delete;

}; // class StageProcFpu

template <typename DT>
class StageProcFpu <0, DT>
{

public:

	static hiir_FORCEINLINE void
	               process_sample_pos (const int nbr_coefs, DT &spl_0, DT &spl_1, StageDataFpu <DT> *stage_arr);
	static hiir_FORCEINLINE void
	               process_sample_neg (const int nbr_coefs, DT &spl_0, DT &spl_1, StageDataFpu <DT> *stage_arr);

private:

	               StageProcFpu ()                                  = delete;
	               StageProcFpu (const StageProcFpu <0, DT> &other) = delete;
	               StageProcFpu (StageProcFpu <0, DT> &&other)      = delete;
	               ~StageProcFpu ()                                 = delete;
	StageProcFpu <0, DT> &
	               operator = (const StageProcFpu <0, DT> &other)   = delete;
	StageProcFpu <0, DT> &
	               operator = (StageProcFpu <0, DT> &&other)        = delete;
	bool           operator == (const StageProcFpu <0, DT> &other)  = delete;
	bool           operator != (const StageProcFpu <0, DT> &other)  = delete;


}; // class StageProcFpu

template <typename DT>
class StageProcFpu <1, DT>
{

public:

	static hiir_FORCEINLINE void
	               process_sample_pos (const int nbr_coefs, DT &spl_0, DT &spl_1, StageDataFpu <DT> *stage_arr);
	static hiir_FORCEINLINE void
	               process_sample_neg (const int nbr_coefs, DT &spl_0, DT &spl_1, StageDataFpu <DT> *stage_arr);

private:

	               StageProcFpu ()                                  = delete;
	               StageProcFpu (const StageProcFpu <1, DT> &other) = delete;
	               StageProcFpu (StageProcFpu <1, DT> &&other)      = delete;
	               ~StageProcFpu ()                                 = delete;
	StageProcFpu <1, DT> &
	               operator = (const StageProcFpu <1, DT> &other)   = delete;
	StageProcFpu <1, DT> &
	               operator = (StageProcFpu <1, DT> &&other)        = delete;
	bool           operator == (const StageProcFpu <1, DT> &other)  = delete;
	bool           operator != (const StageProcFpu <1, DT> &other)  = delete;


}; // class StageProcFpu



}  // namespace hiir



#include "hiir/StageProcFpu.hpp"



#endif   // hiir_StageProc_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
