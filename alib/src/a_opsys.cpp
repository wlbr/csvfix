//---------------------------------------------------------------------------
// a_opsys.cpp
//
// operating system specific functions
//
// Copyright (C) 2006 Neil Butterworth
//---------------------------------------------------------------------------

#include <windows.h>
#include "a_opsys.h"
using std::string;

namespace ALib {

//----------------------------------------------------------------------------

string ExePath() {
    char buffer[MAX_PATH];
    GetModuleFileName( NULL, buffer, MAX_PATH );
    string::size_type pos = string( buffer ).find_last_of( "\\/" );
    return string( buffer ).substr( 0, pos);
}

//---------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_opsys" );

DEFTEST( DirTest ) {
	string s = ExePath();
	//cout << s << endl;
}
#endif

// end

