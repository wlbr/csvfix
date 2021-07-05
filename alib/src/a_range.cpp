//---------------------------------------------------------------------------
// a_range.cpp
//
// range is template class - this is just for testing
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifdef ALIB_TEST

#include "a_range.h"
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_range" );

DEFTEST( RangeTest ) {
	Range <int> r( 10, 20 );
	FAILNE( r.Begin(), 10 );
	FAILNE( r.End(), 20 );
	FAILIFM( r.Contains( 2 ), "bad contains A" );
	FAILIFM( r.Contains( 100 ), "bad contains B" );
	FAILIFM( ! r.Contains( 13 ), "bad contains C" );
	FAILIFM( ! r.Contains( 10 ), "bad contains D" );
	FAILIFM( ! r.Contains( 20 ), "bad contains E" );
}

DEFTEST( Str2Range ) {
	string s = "10:20";
	Range <int> r( s );
	FAILNE( r.Begin(), 10 );
	FAILNE( r.End(), 20 );
	FAILIFM( r.Contains( 2 ), "bad contains A" );
	FAILIFM( r.Contains( 100 ), "bad contains B" );
	FAILIFM( ! r.Contains( 13 ), "bad contains C" );
	FAILIFM( ! r.Contains( 10 ), "bad contains D" );
	FAILIFM( ! r.Contains( 20 ), "bad contains E" );
}

DEFTEST( Expand ) {
	Range <int> r( 10, 20 );
	vector <int> v;
	r.ExpandInto( v );
	FAILNE( v.size(), 11 );
	FAILNE( v[1], 11 );
}


#endif
