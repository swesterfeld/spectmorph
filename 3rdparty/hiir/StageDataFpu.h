/*****************************************************************************

        StageDataFpu.h
        Author: Laurent de Soras, 2020

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://www.wtfpl.net/ for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (hiir_StageDataFpu_HEADER_INCLUDED)
#define hiir_StageDataFpu_HEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace hiir
{



template <typename DT>
class StageDataFpu
{

public:

   DT             _coef = 0;  // a_{n-2}
   DT             _mem  = 0;  // y of the stage

}; // class StageDataFpu



}  // namespace hiir



#endif   // hiir_StageDataFpu_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
