//---------------------------------------------------------------------------
// a_math.cpp
//
// math stuff for alib
//
// Copyright (C) 2006 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_math.h"
#include <cmath>

namespace ALib {

//----------------------------------------------------------------------------
// Round number up
//----------------------------------------------------------------------------

double Round( double d ) {
	return std::floor( d + 0.5 );
}


//---------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_math" );

DEFTEST( RoundTest ) {
	FAILNE( Round( 0.2 ), 0 );
	FAILNE( Round( 0.5 ), 1 );
	FAILNE( Round( 0.72 ), 1 );
	FAILNE( Round( 5.5), 6 );
}

DEFTEST( InRangeTest ) {
	FAILIF( InRange( 42, 1, 10 ))
	FAILNOT( InRange( 7, 1, 10));
}

#endif

//----------------------------------------------------------------------------

// end

