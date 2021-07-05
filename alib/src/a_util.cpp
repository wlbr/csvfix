//---------------------------------------------------------------------------
// a_util.cpp
//
// programming utilities
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_util.h"
#include <iomanip>

using std::string;

namespace ALib {

//----------------------------------------------------------------------------

void HexDump( const void * p, unsigned int n, std::ostream & os  ) {

	const char * cp = reinterpret_cast<const char *>(p);
	for ( unsigned int  i = 0; i < n; i++ ) {
		os << std::hex << std::setw(2) << std::setfill( '0' )
			<< int(cp[i]) << " ";
		if ( (i && ( i % 16 == 0)) || i == n - 1 ) {
			os << "\n";
		}
	}
}

//----------------------------------------------------------------------------

}  // namespace


// end
