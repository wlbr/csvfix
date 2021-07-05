//------------------------------------------------------------------------
// a_str.h
//
// alib string utilities
//
// Copyright (C) 2008 Neil Butterworth
//------------------------------------------------------------------------

#ifndef INC_A_STR_H
#define INC_A_STR_H

#include "a_base.h"
#include <climits>
#include <sstream>
#include <bitset>

//------------------------------------------------------------------------
// Begin ALib stuff
//------------------------------------------------------------------------

namespace ALib {

//------------------------------------------------------------------------
// Shorter versions of standard names
//------------------------------------------------------------------------

typedef std::string::size_type STRSIZE;
typedef std::string::size_type STRPOS;
const STRPOS STRNPOS = std::string::npos;

//------------------------------------------------------------------------
// Convert any streamable type to string
//------------------------------------------------------------------------

template <class TYPE> std::string Str( const TYPE & t ) {
	std::ostringstream os;
	os << t;
	return os.str();
}

//---------------------------------------------------------------------------
// Return a string that consists of a single NUL byte
//---------------------------------------------------------------------------

std::string NULByteStr();

//------------------------------------------------------------------------
// Convert string using function that looks like std::toupper()
//------------------------------------------------------------------------

template <class CFUNC> std::string Convert( const std::string & s,
													CFUNC f ) {
	std::string t(s);
	const unsigned int n = t.size();
	for ( unsigned int i = 0; i < n; i++ ) {
		t[i] = f( t[i] );
	}
	return t;
}

//------------------------------------------------------------------------
// Frequently used conversions
//------------------------------------------------------------------------

std::string Upper( const std::string & s );
std::string Lower( const std::string & s );

//---------------------------------------------------------------------------
// Convrt to capitalised first char of each word
//---------------------------------------------------------------------------

std::string Capitals( const std::string & s );

//---------------------------------------------------------------------------
// See if string is prefixed or suffixed by a string
//---------------------------------------------------------------------------

bool IsPrefixed( const std::string & s, const std::string & pre );
bool IsSuffixed( const std::string & s, const std::string & suf );

//---------------------------------------------------------------------------
// Does string contain substring?
//---------------------------------------------------------------------------

bool HasSubstr( const std::string & s, const std::string & subs );

//---------------------------------------------------------------------------
// Get first/last character from a string or NUL char if string is empty.
//---------------------------------------------------------------------------

char StrFirst( const std::string & s );
char StrLast( const std::string &  s );

//------------------------------------------------------------------------
// Trimming whitespace
//------------------------------------------------------------------------

std::string Trim( const std::string & s );
std::string LTrim( const std::string & s );
std::string RTrim( const std::string & s );

//----------------------------------------------------------------------------
// Remove N trailing characters
//----------------------------------------------------------------------------

void Trunc( std::string & s, unsigned int n );

//------------------------------------------------------------------------
// Wrap string in quotes in various ways
//------------------------------------------------------------------------

std::string SQuote( const std::string & s, const std::string & esc = "" );
std::string DQuote( const std::string & s, const std::string & esc = "" );
std::string SQLQuote( const std::string & s, char qchar = '\'' );
std::string UnQuote( const std::string & s );
bool IsQuoted( const std::string & s );
std::string SQLStr( const std::string & s );

//----------------------------------------------------------------------------
// Get indexed character of string or 0 if past end
//----------------------------------------------------------------------------

char Peek( const std::string & s, unsigned int i );

//----------------------------------------------------------------------------
// Get string of length n at position i or empty string if not enough chars.
//----------------------------------------------------------------------------

std::string PeekStr( const std::string & s, unsigned int i );

//----------------------------------------------------------------------------
// See if string at position i in s is str.
//----------------------------------------------------------------------------

bool PeekStr( const std::string & s, unsigned int i,
					const std::string & str );

//------------------------------------------------------------------------
// Add double quotes around string and quote any contained double quotes
//------------------------------------------------------------------------

std::string CSVQuote( const std::string & s );

//---------------------------------------------------------------------------
// Pad integer with leasing zeros
//---------------------------------------------------------------------------

std::string ZeroPad( unsigned int i, unsigned int width );
std::string LeftPad( const std::string & s, unsigned int width, char c = ' ' );
std::string RightPad( const std::string & s, unsigned int width, char c = ' ' );
std::string Centre( const std::string & s, unsigned int width, char c = ' ' );

//---------------------------------------------------------------------------
// Escape characters in string
//---------------------------------------------------------------------------

std::string Escape( const std::string & s, const std::string & chars,
						const std::string & esc );

//---------------------------------------------------------------------------
// Unescape \n, \t and \r
//---------------------------------------------------------------------------

std::string UnEscape( const std::string & s );


//----------------------------------------------------------------------------
// Add to string iff the string is or is not empty - useful for making
// comma-separated lists etc.
//----------------------------------------------------------------------------

std::string AddIfEmpty( std::string & s, const std::string & add );
std::string AddIfNotEmpty( std::string & s, const std::string & add );

//------------------------------------------------------------------------
// Testing functions
//------------------------------------------------------------------------

bool IsEmpty( const std::string & s );
bool IsIdentifier( const std::string & s );
bool Equal( const std::string & lhs, const std::string & rhs );
bool IsIdent( char c );
bool IsIdentBegin( char c );

//------------------------------------------------------------------------
// Numeric testing
//------------------------------------------------------------------------

bool IsNumber( const std::string & s );
bool IsInteger( const std::string & s );
bool IsReal( const std::string & s );

//------------------------------------------------------------------------
// Numeric conversions
//------------------------------------------------------------------------

long ToInteger( const std::string & s, const std::string & emsg = "" );
double ToReal( const std::string & s, const std::string & emsg = "" );

//---------------------------------------------------------------------------
// Convert strings to and from boolean. Booleans are encoded as
// 'Y' or 'N'. For ToBool(), an exception is thrown if the string does
// not contain 'Y' or 'N'.
//---------------------------------------------------------------------------

bool ToBool( const std::string & s, const std::string & emsg = "" );
std::string FromBool( bool b );
bool IsBool( const std::string & s );

//------------------------------------------------------------------------
// Squeeze out chars
//------------------------------------------------------------------------

std::string Squeeze( const std::string & s, const std::string & out );

//------------------------------------------------------------------------
// Replace substring (no regexp)
//------------------------------------------------------------------------

std::string Replace( const std::string & s,
						const std::string & target,
						const std::string & with,
						unsigned int count = UINT_MAX );

//---------------------------------------------------------------------------
// Format using printf() formatters
//---------------------------------------------------------------------------

std::string Format( const char * fmt, ... );

//----------------------------------------------------------------------------
// Convert string containing newlines to vector (or anything that supports
// push_back).
//----------------------------------------------------------------------------

template <typename PBTYPE>
void StrToVec( const std::string & s, PBTYPE & v, char nlc = '\n' ) {
	std::string line;
	for ( unsigned int i = 0; i < s.size(); i++ ) {
		char c = s[i];
		if ( c == nlc ) {
			v.push_back( line );
			line = "";
		}
		else {
			line += c;
		}
	}
	if ( line != "" ) {
		v.push_back( line );
	}
}

//------------------------------------------------------------------------
// Tokenise strings
//------------------------------------------------------------------------

unsigned int Split( const std::string & s, char sep,
						std::vector <std::string> & tokens );

unsigned int Split( const std::string & s, const std::string & sep,
						std::vector <std::string> & tokens );

unsigned int Words( const std::string & s,
						std::vector <std::string> & words );


//---------------------------------------------------------------------------
// Class wrapper for comma-separated list tokenisation
//---------------------------------------------------------------------------

class CommaList {

	public:

		CommaList( const std::string & s = "" );

		unsigned int Set( const std::string & s );
		unsigned int Size() const;
		const std::string & At( unsigned int i ) const;
		const std::vector <std::string> & Items() const;
		const CommaList & Append ( const CommaList & cl );
		const CommaList & Append ( const std::string & s );
		bool Contains( const std::string & s ) const;
		int Index( const std::string & s ) const;

	private:

		std::vector <std::string> mItems;
};

//---------------------------------------------------------------------------
// Enum used to indicate case sensitivity of comparisons etc.
//---------------------------------------------------------------------------

enum CaseSensitive {
	IgnoreCase = 0, RespectCase = 1
};

//---------------------------------------------------------------------------
// Comparison function as for strcmp
//---------------------------------------------------------------------------

int Cmp( const std::string & s1, const std::string & s2, CaseSensitive cs );

//---------------------------------------------------------------------------
// Class used for case insensitive maps etc.
//---------------------------------------------------------------------------

template <CaseSensitive CS> class Less {

	public:

		bool operator()( const std::string & s1,
						const std::string & s2 ) const {
			return Cmp( s1, s2, CS ) < 0;
		}

};


//---------------------------------------------------------------------------
// Search lists of strings
//---------------------------------------------------------------------------

bool In( const std::string & s, const std::string & commalist,
									CaseSensitive cs = IgnoreCase );
bool In( const std::string & s, const std::vector <std::string> & list,
									CaseSensitive cs = IgnoreCase  );
bool In( const std::string & s, CaseSensitive cs,
									const char * list, ... );


//---------------------------------------------------------------------------
// Search list of pointers to NTS.
//---------------------------------------------------------------------------

int Find( const char * list[], const std::string & val );


//---------------------------------------------------------------------------
// Character ranges like "A-Za-z"
//---------------------------------------------------------------------------

class CharRange {

	public:

		CharRange( const std::string & rs = "" );

		std::string Str() const;
		bool Contains( unsigned char c ) const;

	private:

		void DoSpecial( const std::string & rs, unsigned int & pos );
		void ProcessChar( const std::string & rs, unsigned int & pos );
		void MakeHyphenRange( const std::string & rs, unsigned int & pos );
		void SetChar( unsigned int c );

		enum { SETSIZE = 128 };
		std::bitset <SETSIZE> mCharFlags;
		bool mInvert;
};

//----------------------------------------------------------------------------
// Convert  to binary bit pattern
//----------------------------------------------------------------------------

std::string BinStr( unsigned int n, char pad = 0 );

//------------------------------------------------------------------------
// end ALib stuff
//------------------------------------------------------------------------

}

#endif
