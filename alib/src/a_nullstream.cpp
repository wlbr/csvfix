//---------------------------------------------------------------------------
// a_nullstream.cpp
//
// Null stream test stuff - all code is in header
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_nullstream.h"

using std::string;

#ifdef ALIB_TEST

#include "a_range.h"
#include "a_myth.h"

using namespace ALib;
using namespace std;

DEFSUITE( "a_nullstream" );

static void f( ostream & os ) {
	os << "bar" << 456 << endl;
}

DEFTEST( NullStreamTest ) {
	NullStream ns;
	ns << "foo" << 123 << endl;
	f( ns );
}

#endif


