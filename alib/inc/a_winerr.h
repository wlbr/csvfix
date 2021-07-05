//----------------------------------------------------------------------------
// a_winerr.h
//
// report win32 errors
//
// Copyright (C) 20010 Neil Butterworth
//----------------------------------------------------------------------------


#ifndef INC_A_WINERR_H
#define INC_A_WINERR_H

#include "a_win.h"
#include <string>

namespace ALib {

std::string LastWinError();
std::string WinErrorToStr( DWORD ec );

}


#endif
