//----------------------------------------------------------------------------
// a_winerr.cpp
//
// report win32 errors
//
// Copyright (C) 20010 Neil Butterworth
//----------------------------------------------------------------------------


#include "a_winerr.h"
#include "a_str.h"

namespace ALib {

#ifdef ALIB_WINAPI

std::string WinErrorToStr( DWORD ec ) {

	char * lpMsgBuf;
	::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					ec,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR) &lpMsgBuf,
					0,
					NULL );
	std::string msg ( lpMsgBuf );
	::LocalFree( lpMsgBuf );
	return ALib::Trim( msg );
}


std::string LastWinError() {
	return WinErrorToStr( ::GetLastError() );
}


}	// ALib

//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
#include "a_str.h"

using namespace ALib;
using namespace std;

DEFSUITE( "a_winerr" );

DEFTEST( ErrMsgTest ) {
	HANDLE h = ::GetClipboardData( 66666 );		// bad call
	FAILNE( h, 0 );
	string emsg = LastWinError();
//	std::cout << "["  << emsg << "]" << std::endl;
	// might be different in different Windows versions
	string expected = "Thread does not have a clipboard open.";
	FAILNE( emsg, expected );
}

#endif

//----------------------------------------------------------------------------


#endif


//----------------------------------------------------------------------------




