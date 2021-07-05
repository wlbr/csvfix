//----------------------------------------------------------------------------
// a_win.h
//
// Include Windows headers and define ALIB_WIN if we are on windows.
// Any Windows specific code should include this.
//
// Copyright (C) 2010 Neil Butterworth
//----------------------------------------------------------------------------

#ifndef INC_ALIB_WIN_H
#define INC_ALIB_WIN_H

#ifdef WINNT
	#undef ALIB_WINAPI
	#define ALIB_WINAPI
	#include <windows.h>
#else
	#undef ALIB_WINAPI
#endif

#endif
