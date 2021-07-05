//------------------------------------------------------------------------
// a_debug.h
//
// Debug stuff for alib - windows only
//
// Copyright (C) 2008 Neil Butterworth
//------------------------------------------------------------------------

#ifndef INC_A_DEBUG_H
#define INC_A_DEBUG_H

#include "a_win.h"
#include <sstream>

namespace ALib {
	extern std::ostringstream debug_os_;
}

//----------------------------------------------------------------------------

#ifdef WINNT

#define DBGMSG( msg_ ) 												\
{																	\
	ALib::debug_os_.str("");										\
	ALib::debug_os_ << __FILE__ << " (" << __LINE__ << ") ";		\
	ALib::debug_os_ << msg_;										\
	OutputDebugString( ALib::debug_os_.str().c_str() );				\
}


#define DBGBOX( msg_ )												\
{																	\
	ALib::debug_os_.str("");										\
	ALib::debug_os_ << __FILE__ << " (" << __LINE__ << ") ";		\
	ALib::debug_os_ << msg_;										\
	MessageBox( 0, ALib::debug_os_.str().c_str(),					\
				  "ALib Debug", MB_ICONASTERISK | MB_OK );			\
}

#endif

//----------------------------------------------------------------------------

#endif

