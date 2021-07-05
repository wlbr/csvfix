//---------------------------------------------------------------------------
// a_chsrc.h
//
// Character source for parsers/lexers etc.  This handles character escaping 
// using the backslash, provides safe end of string checking etc.
//
// Copyright (C) 2006 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_CHSRC_H
#define INC_A_CHSRC_H

#include "a_base.h"
#include <string>

//---------------------------------------------------------------------------
// Begin ALib stuff
//---------------------------------------------------------------------------

namespace ALib {

//------------------------------------------------------------------------

class CharSource {

	CANNOT_COPY( CharSource );

	public:

		class Char {

			public:

				Char( char c = 0, bool escaped = false ) 
					: mChar( c ), mEscaped( escaped ) {
				}

				char Value() const { return mChar; }
				bool Escaped() const { return mEscaped; }

			private:				

				char mChar;
				bool mEscaped;
		};

		CharSource( const std::string & s );

		bool Next( Char & c );
		bool Peek( Char & c, unsigned int i = 0 ) const;
		
		bool AtEnd() const;
		void Reset();

	private:

		const std::string & mStr;
		unsigned int mPos;

};

//---------------------------------------------------------------------------
// Wnd ALib stuff
//---------------------------------------------------------------------------

}

#endif

