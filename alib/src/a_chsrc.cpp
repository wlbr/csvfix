//---------------------------------------------------------------------------
// a_chsrc.cpp
//
// character source for parsers lexets etc.
//
// Copyright (C) 2006 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_chsrc.h"
#include "a_except.h"
using std::string;

//---------------------------------------------------------------------------
// Begin ALib stuff
//---------------------------------------------------------------------------

namespace ALib {


//---------------------------------------------------------------------------
//  char to use for escapes
//---------------------------------------------------------------------------

const char ESCAPE = '\\';


//---------------------------------------------------------------------------
// set up source characters
//---------------------------------------------------------------------------

CharSource :: CharSource( const string & s ) 
			: mStr( s ), mPos( 0 ) {
}

//---------------------------------------------------------------------------
// Get next character into c, advancing scan, returns success indicator
//---------------------------------------------------------------------------

bool CharSource :: Next( CharSource::Char & c ) {
	if ( Peek( c ) ) {
		mPos += c.Escaped() ? 2 : 1;
		return true;
	}
	else {
		return false;
	}
}

//---------------------------------------------------------------------------
// Look at next character next character without advancing scan
// Returns false if we are at end of source
//---------------------------------------------------------------------------

bool CharSource :: Peek( CharSource::Char & c, unsigned int i ) const {
	unsigned int pos = mPos + i ;
	if ( pos >= mStr.size() ) {
		c = Char( 0, false );
		return false;
	}
	char a = mStr[ pos ];
	bool escaped = false;
	if ( a == ESCAPE ) {
		if ( pos + 1 == mStr.size() ) {
			ATHROW( "Invalid escape at end of string" );
		}
		a = mStr[ pos + 1 ];
		escaped = true;
	}
	c = Char( a, escaped );
	return true;
}

//---------------------------------------------------------------------------
// Any more characters?
//---------------------------------------------------------------------------

bool CharSource :: AtEnd() const {
	return mPos >= mStr.size();
}

//---------------------------------------------------------------------------
// Reset scan position
//---------------------------------------------------------------------------

void CharSource :: Reset() {
	mPos = 0;
}

//---------------------------------------------------------------------------
// End ALib stuff
//---------------------------------------------------------------------------

}

// end

