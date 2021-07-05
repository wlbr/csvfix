//----------------------------------------------------------------------------
// a_io.cpp
//
// I/O stuff for alib
//
// Copyright (C) 20010 Neil Butterworth
//----------------------------------------------------------------------------

#include "a_io.h"


//----------------------------------------------------------------------------
// Testing
//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
#include "a_str.h"
#include "a_debug.h"
#include <vector>
using namespace ALib;
using namespace std;

DEFSUITE( "a_io" );

DEFTEST( RedirectionTest1 ) {
	ifstream ifs( "test/lines3.txt");
	FAILIF( (! ifs.is_open()) );
	InputRedirector ir( std::cin, ifs );
	string line;
	getline( std::cin, line );
	FAILNE( line, "line 1" );
}



#endif

