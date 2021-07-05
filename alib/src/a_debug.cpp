//---------------------------------------------------------------------------
// a_debug.cpp
//
// debug stuff
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------


#include "a_debug.h"

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

namespace ALib {
	std::ostringstream debug_os_;
}

/*
DEFSUITE( "a_debug" );

DEFTEST( DBGTest ) {
	DBG( "This is a DBG test - the meaning is " << 42 );
	DBG( "Another line of output" );
}

DEFTEST( DBGBox ) {
	DBGBOX( "The meaning is " << 42 );
}
*/

#endif


// end
