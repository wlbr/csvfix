//----------------------------------------------------------------------------
// a_matrix.cpp
//
// simole 2d matrix
//
// Copyright (C) 20010 Neil Butterworth
//----------------------------------------------------------------------------

#include "a_matrix.h"


#ifdef ALIB_TEST
#include "a_myth.h"
#include <iostream>
using namespace ALib;
using namespace std;

DEFSUITE( "a_matrix" );

DEFTEST( simple ) {
	Matrix2D <int> m( 3, 4 );
	m( 2, 3 ) = 42;
	FAILIF( m.Width() != 3 );
	FAILIF( m.Height() != 4 );
	FAILIF( m(2,3) != 42 );
	FAILIF( m(0,0 ) != 0 );
}

/*
DEFTEST( Dump ) {
	Matrix2D <int> m( 3, 4 );
	m( 1, 2 ) = 1;
	m.DumpOn( cout );
}
*/

#endif

