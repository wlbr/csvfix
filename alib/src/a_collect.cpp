//----------------------------------------------------------------------------
// a_collect.cpp
//
// tests for collection templates
//
// Copyright (C) 20010 Neil Butterworth
//----------------------------------------------------------------------------

#include "a_collect.h"

#ifdef ALIB_TEST
#include "a_myth.h"
#include <iostream>
#include <vector>
using namespace ALib;
using namespace std;

DEFSUITE( "a_collect" );

DEFTEST( append ) {

	vector <int> a, b;
	a.push_back( 1 );
	a.push_back( 2 );
	b.push_back( 3 );
	b.push_back( 4 );

	a += b;

	FAILIF( a.size() != 4 );
	FAILIF( a[2] != 3 );
}


#endif

