//----------------------------------------------------------------------------
// a_strscan.cpp
//
// simple string scanner
//
// Copyright (C) 20010 Neil Butterworth
//----------------------------------------------------------------------------

#include "a_base.h"
#include "a_strscan.h"
using std::string;

namespace ALib {

//----------------------------------------------------------------------------
// Construct in invalid dstate - Next() must be called before anything else.
//----------------------------------------------------------------------------

StrScanner :: StrScanner( const string & s )
	: mStr( s ), mPos( 0 ), mCurrent( EOS ) {
}

//----------------------------------------------------------------------------
// Return current character, advancing scan.
//----------------------------------------------------------------------------

int StrScanner :: Next() {
	if ( mPos >= mStr.size() ) {
		mCurrent = EOS;
	}
	else {
		mCurrent = mStr[mPos++];
	}
	return mCurrent;
}

//----------------------------------------------------------------------------
// Get last character returned by Next()
//----------------------------------------------------------------------------


int StrScanner :: Current() const {
	CheckNextCalled();
	return mCurrent;
}

//----------------------------------------------------------------------------
// Look at next character that calling Next n timed would return - default
// is zero times.
//----------------------------------------------------------------------------

int  StrScanner :: Peek( unsigned int  n ) const {
	CheckNextCalled();
	if ( n + mPos >= mStr.size() ) {
		return EOS;
	}
	else {
		return mStr[ n + mPos ];
	}
}

//----------------------------------------------------------------------------
// Helper to check that we are in valid state - Next has been called
//----------------------------------------------------------------------------

void StrScanner :: CheckNextCalled() const {
	if ( mPos == 0 ) {
		ATHROW( "Alib::StrScanner - Next not called" );
	}
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------
// tests
//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_strscan" );

DEFTEST( Construct ) {
	StrScanner s( "foobar" );
}

DEFTEST( Next ) {
	StrScanner s( "bar" );
	char c = s.Next();
	FAILIF( c != 'b' );
	c = s.Next();
	FAILIF( c != 'a' );
	FAILIF( s.Current() != 'a' );
	c = s.Next();
	FAILIF( c != 'r' );
	int n = s.Next();
	FAILIF( n !=  StrScanner::EOS );
}

DEFTEST( Peek ) {
	StrScanner s( "bar" );
	char c = s.Next();
	FAILIF( c != 'b' );
	c = s.Peek();
	FAILIF( c != 'a' );
	c = s.Peek(1);
	FAILIF( c != 'r' );
	int n = s.Peek(2);
	FAILIF( n !=  StrScanner::EOS );

}

#endif

// end
