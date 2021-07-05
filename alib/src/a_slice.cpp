//---------------------------------------------------------------------------
// a_slice.cpp
//
// slice tests - all code is templated in header
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifdef ALIB_TEST

#include "a_base.h"
#include "a_slice.h"

using std::string;
using std::vector;

#include "a_myth.h"
using namespace std;
using namespace ALib;


DEFSUITE( "a_slice" );

DEFTEST( SliceTest ) {

	Slice <int> s;
	FAILNE( s.size(), 0 );

	vector <int> e;
	Slice <int> es( e );
	FAILNE( es.size(), 0 );
	FAILIF( es.begin() != es.end() );

	vector <int> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);
	v.push_back(5);
	v.push_back(6);
	v.push_back(7);
	Slice <int> sv( v );
	FAILNE( sv.size(), 7 );
	FAILNE( sv.at(1), 2 );
	FAILNE( sv[1], 2 );


	Slice <int> s2( v, 1, 3 );
	FAILNE( s2.size(), 3 );
	FAILNE( s2.at(1), 3 );

	Slice <int> s3(sv, 1, 3 );
	FAILNE( s3.size(), 3 );
	FAILNE( s3.at(1), 3 );

}

#endif

// end

