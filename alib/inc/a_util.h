//---------------------------------------------------------------------------
// a_util.h
//
// programming support utilities
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_TIME_H
#define INC_A_TIME_H

#include "a_base.h"
#include <iostream>

namespace ALib {

//------------------------------------------------------------------------

void HexDump( const void * p, unsigned int n, std::ostream & os = std::cout );

//----------------------------------------------------------------------------

}  // namespace


#endif

