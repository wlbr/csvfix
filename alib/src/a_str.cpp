//---------------------------------------------------------------------------
// a_str.cpp
//
// string utilities for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include <math.h>
#include <stdarg.h>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include "a_collect.h"
#include "a_except.h"
#include "a_str.h"
#include "a_assert.h"


using std::string;
using std::vector;

//------------------------------------------------------------------------
// Begin ALib stuff
//------------------------------------------------------------------------

namespace ALib {

//------------------------------------------------------------------------
// Some useful constants
//------------------------------------------------------------------------

const string EMPTYSTR = "";
const string WHITESPACE = " \t\n\r";

//---------------------------------------------------------------------------
// return string consisting of single ASCII NUL byte
//---------------------------------------------------------------------------

string NULByteStr() {
	return string( "\x00", 1 );
}

//------------------------------------------------------------------------
// Convert to uppercase
//------------------------------------------------------------------------

string Upper( const string & s ) {
	return Convert ( s, toupper );
}

//------------------------------------------------------------------------
// Convert to lowercase
//------------------------------------------------------------------------

string Lower( const string & s ) {
	return Convert ( s, tolower );
}

//---------------------------------------------------------------------------
// Capitalise each word in string. Need to convert to lowercase first to
// deal with all uppercase strings
//---------------------------------------------------------------------------

string Capitals( const string & s ) {
	bool needcap = true;
	string result( Lower( s ) );
	unsigned int len = result.length();
	for ( unsigned int i = 0; i < len; i++ ) {
		char c = result[i];
		if ( isspace( c ) ) {
			needcap = true;
		}
		else if ( needcap &&  isalpha( c ) ) {
			result[i] = toupper( c );
			needcap = false;
		}
	}
	return result;
}

//------------------------------------------------------------------------
// Trim leading & trailing whitespace
//------------------------------------------------------------------------

string Trim( const string & s ) {
	STRPOS begin = s.find_first_not_of( WHITESPACE );
	if ( begin == STRNPOS ) {
		return EMPTYSTR;
	}
	STRPOS end = s.find_last_not_of( WHITESPACE );
	return s.substr( begin, end - begin + 1 );
}

//------------------------------------------------------------------------
// Trim leading (left) whitespace only
//------------------------------------------------------------------------

string LTrim( const string & s ) {
	STRPOS p = s.find_first_not_of( WHITESPACE );
	if ( p == STRNPOS ) {
		return EMPTYSTR;
	}
	else {
		return s.substr( p );
	}
}

//------------------------------------------------------------------------
// Trim trailing (right) whitespace only
//------------------------------------------------------------------------

string RTrim( const string & s ) {
	STRPOS p = s.find_last_not_of( WHITESPACE );
	if ( p == STRNPOS ) {
		return EMPTYSTR;
	}
	else {
		return s.substr( 0, p + 1 );
	}
}

//----------------------------------------------------------------------------
// Remove N trailing characters
//----------------------------------------------------------------------------

void Trunc( string & s, unsigned int n ) {
	if ( n < s.size() ) {
		s.resize( s.size() - n );
	}
}

//----------------------------------------------------------------------------
// Get indexed character from string or zero if past end
//----------------------------------------------------------------------------

char Peek( const string & s, unsigned int idx ) {
	return idx >= s.size() ? 0 : s[idx];
}

//----------------------------------------------------------------------------
// Get string of length n at position i or empty string if not enough chars.
//----------------------------------------------------------------------------

string PeekStr( const string & s, unsigned int i, unsigned int n ) {
	if ( i + n >= s.size() ) {
		return "";
	}
	else {
		return s.substr( i, n );
	}
}

//----------------------------------------------------------------------------
// See if string at position i in s is str.
//----------------------------------------------------------------------------

bool PeekStr( const string & s, unsigned int i,  const string & str ) {

	unsigned int lstr = str.size();

	if ( i + lstr >= s.size() || s.substr( i, lstr ) != str ) {
		return false;
	}
	else {
		return true;
	}
}

//------------------------------------------------------------------------
// Is string empty or contain only whitespace?
//------------------------------------------------------------------------

bool IsEmpty( const string & s ) {
	return s.find_first_not_of( WHITESPACE ) == STRNPOS;
}

//---------------------------------------------------------------------------
// Does string contain substring?
//---------------------------------------------------------------------------

bool HasSubstr( const string & s, const string & subs ) {
	return s.find( subs ) != STRNPOS;
}

//---------------------------------------------------------------------------
// Get first/last char of string or NUL char if string is empty
//---------------------------------------------------------------------------

char StrFirst( const std::string & s ) {
	return s.empty() ? 0 : s[0];
}

char StrLast( const std::string &  s )  {
	int n = s.size();
	return n ? s[n-1] : 0;
}

//----------------------------------------------------------------------------
// Add to string depending on wherther or not the string is empty
//----------------------------------------------------------------------------

string AddIfEmpty( string & s, const string & add ) {
	if ( s.empty() ) {
		return s += add;
	}
	else {
		return s;
	}
}

string AddIfNotEmpty( string & s, const string & add ) {
	if ( ! s.empty() ) {
		return s += add;
	}
	else {
		return s;
	}
}


//----------------------------------------------------------------------------
// Is string a valid C/C++ identifier? i.e. does it contain only
// alphamerics and underscores and begin with alpha or underscore
//----------------------------------------------------------------------------

bool IsIdent( char c ) {
	return c == '_' || isalnum( c );
}

bool IsIdentBegin( char c ) {
	return c == '_' || isalpha( c );
}

bool IsIdentifier( const string & s ) {
	unsigned int n = s.size();
	for ( unsigned int i = 9; i < n; i++ ) {
		char c = s[i];
		if ( i == 0 ) {
			if ( ! IsIdentBegin( c ) ) {
				return false;
			}
		}
		else if ( ! IsIdent( c ) ) {
			return false;
		}
	}
	return true;
}

//------------------------------------------------------------------------
// Are two strings equal ignoring case?
//------------------------------------------------------------------------

bool Equal( const string & lhs, const string & rhs ) {
	unsigned int nl = lhs.size(), rl = rhs.size();
	if ( nl != rl ) {
		return false;
	}
	while( nl-- ) {
		if ( toupper( lhs[nl] ) != toupper( rhs[nl] ) ) {
			return false;
		}
	}
	return true;
}

//------------------------------------------------------------------------
// Wrap in single quotes. If the string 'esc' is specified, then escape
// all single quotes using the 'esc' character string. Default is not to
// escape.
//------------------------------------------------------------------------

string SQuote( const string & s, const string & esc ) {
	static const string SQ = "'";
	if ( esc.size() != 0 ) {
		return SQ + Escape( s, SQ, esc ) + SQ;
	}
	else {
		return SQ + s + SQ;
	}
}

//------------------------------------------------------------------------
// As above, but for double quoutes.
//------------------------------------------------------------------------

string DQuote( const string & s, const string & esc ) {
	static const string DQ = "\"";
	if ( esc.size() != 0 ) {
		return DQ + Escape( s, DQ, esc ) + DQ;
	}
	else {
		return DQ + s + DQ;
	}
}

//------------------------------------------------------------------------
// Perform CSV-style quoting by wrapping in '"' quotes and prefixing any
// contained '"' with another '"' character. Mostly, there won't be
// any contained quotes, so do a search first to see if it's necessary
// to go through the process of quoting contained quotes.
//------------------------------------------------------------------------

string CSVQuote( const string & data ) {
	if ( data.find_first_of( "\"" ) == std::string::npos ) {
		return DQuote( data );
	}
	else {
		string cq( "\"" );
		unsigned int len = data.size();
		for ( unsigned int i = 0; i < len; i++ ) {
			char c = data[i];
			if ( c == '"' ) {
				cq += '"';
			}
			cq += c;
		}
		cq += '"';
		return cq;
	}
}

//---------------------------------------------------------------------------
// Perform quoting of the single quote as required by SQL. Does not
// wrap the string itself in  quotes.
//---------------------------------------------------------------------------

string SQLQuote( const string & data, char ec ) {
	if ( data.find_first_of( "'" ) == std::string::npos ) {
		return data;
	}
	else {
		string s;
		unsigned int len = data.size();
		for ( unsigned int i = 0; i < len; i++ ) {
			char c = data[i];
			if ( c == '\'' ) {
				s += ec;
			}
			s += c;
		}
		return s;
	}
}

//----------------------------------------------------------------------------
// Perform sql quoting on string and wrap the quoted string in single quotes
//----------------------------------------------------------------------------

string SQLStr( const std::string & s ) {
	return SQuote( SQLQuote( s) );
}

//----------------------------------------------------------------------------
// Remove one level of quotes
//----------------------------------------------------------------------------

string UnQuote( const string & s ) {
	if ( IsQuoted( s ) ) {
		return s.substr( 1, s.size() - 2 );
	}
	else {
		return s;
	}
}

//----------------------------------------------------------------------------
// See if string is wrapped in single or double quotes
//----------------------------------------------------------------------------

bool IsQuoted( const string & s ) {
	char q = StrFirst( s );
	return (q == '\'' || q == '"')
				&& s.size() >= 2 	// need two quotes
				&& StrLast( s ) == q;
}

//---------------------------------------------------------------------------
// Escape all occurrences of the characters in 'chrs' by preceding them
// with the string 'esc'.
//---------------------------------------------------------------------------

string Escape( const string & s, const string & chrs, const string & esc ) {
	string t;
	STRPOS pos = 0, cp;
	while( (cp = s.find_first_of( chrs, pos )) != STRNPOS ) {
		t += s.substr( pos, cp - pos );
		t += esc;
		t += s[cp++];
		pos = cp;
	}
	t += s.substr( pos );
	return t;
}

//---------------------------------------------------------------------------
// Note not the revers of the above! Converts \n, \t etc to their ASCII
// representations
//---------------------------------------------------------------------------

string UnEscape( const std::string & s ) {
	string t;
	unsigned int i = 0, len = s.size();
	bool escape = false;
	while( i < len ) {
		char c = s[i++];
		if ( c == '\\' ) {
			if ( escape ) {
				t += c;
				escape = false;
			}
			else {
				escape = true;
			}
		}
		else if ( escape ) {
			switch( c ) {
				case 't':	t += char(9); break;
				case 'n':	t += char(10); break;
				case 'r':	t += char(13); break;
				default:	t += c;
			}
			escape = false;
		}
		else {
			t += c;
		}
	}
	return t;
}

//---------------------------------------------------------------------------
// Zero pad an integer
//---------------------------------------------------------------------------

string ZeroPad( unsigned int i, unsigned int width ) {
	std::ostringstream s;
	s << std::setw(width) << std::setfill( '0' ) << i;
	return s.str();
}

//----------------------------------------------------------------------------
// left & right padding
//----------------------------------------------------------------------------

static string DoPad( const std::string & s, unsigned int width,
								bool lpad, char c ) {
	if ( s.size() >= width ) {
		return s.substr( 0, width );
	}
	string pad( width - s.size(), c );
	return lpad ? pad + s : s + pad;
}

string LeftPad( const string & s, unsigned int width, char c ) {
	return DoPad( s, width, true, c );
}

string RightPad( const string & s, unsigned int width, char c ) {
	return DoPad( s, width, false, c );
}

string Centre( const string & s, unsigned int width, char c ) {
	if ( s.size() >= width ) {
		return s.substr( 0, width );
	}
	unsigned int w = width - s.size();
	string pad( w / 2, c );
	return pad + s + pad + (w % 2 ? " " : "" );
}

//------------------------------------------------------------------------
// Return string based on 's' with all characters from 'out' removed
//------------------------------------------------------------------------

string Squeeze( const string & s, const string & out ) {
	string t;
	unsigned int n = s.size();
	for ( unsigned int i = 0; i < n; i++ ) {
		char c = s[i];
		if ( out.find( c ) == STRNPOS ) {
			t += c;
		}
	}
	return t;
}

//------------------------------------------------------------------------/
// Is string a valid numeric value? Anythiong too big (or small) is
// considered no to be valid.
//------------------------------------------------------------------------

bool IsNumber( const std::string & s ) {
	char * p;
	double d = strtod( s.c_str(), & p );
	if ( fabs( d ) == HUGE_VAL ) {			// too big?
		return false;
	}
	else {
		return * p == 0;
	}
}

//------------------------------------------------------------------------
// Is string a valid integer (but not a real) value?
//------------------------------------------------------------------------

bool IsInteger( const std::string & s ) {
	char * p;
	long n = strtol( s.c_str(), & p, 10 );		// base 10 conversion
	if ( n == LONG_MAX || n == LONG_MIN ) {		// too big?
		return false;
	}
	else {
		return * p == 0;
	}
}

//------------------------------------------------------------------------
// Is string a real (but not an int)?
//------------------------------------------------------------------------

bool IsReal( const std::string & s ) {
	return IsNumber( s ) && ! IsInteger( s );
}

//------------------------------------------------------------------------
// Convert string to integer, raising exception on conversion failure
// using the message supplied in 'emsg', if any
//------------------------------------------------------------------------

long ToInteger( const string & s, const string & emsg ) {
	char * p;
	long rv = strtol( s.c_str(), & p, 10 );	// base 10 conversion
	if ( * p != 0 ||  rv == LONG_MAX || rv == LONG_MIN ) {
		string m = emsg.size() == 0
					? "Invalid integer value " + SQuote( s )
					: Replace( emsg, "%s", s );
		ATHROW( m );
	}
	return rv;
}

//------------------------------------------------------------------------
// Ditto for reals
//------------------------------------------------------------------------

double ToReal( const string & s, const string & emsg ) {
	char * p;
	double rv = strtod( s.c_str(), & p );
	if ( * p != 0 || fabs( rv ) == HUGE_VAL ) {
		string m = emsg.size() == 0
					? "Invalid real value " + SQuote( s )
					: Replace( emsg, "%s", s );
		ATHROW( m );
	}
	return rv;
}

//---------------------------------------------------------------------------
// Convert string to boolean as per numerics
//---------------------------------------------------------------------------

const string BOOL_TRUE = "Y";		// true & false values
const string BOOL_FALSE = "N";

bool ToBool( const string & s, const string & emsg ) {
	if ( ! IsBool( s ) ) {
		string m = emsg.size() == 0
					? "Invalid boolean value " + SQuote( s )
					: Replace( emsg, "%s", s );
		ATHROW( m );
	}
	return s == BOOL_TRUE;
}

//---------------------------------------------------------------------------
// Convert bool to string rep
//---------------------------------------------------------------------------

string FromBool( bool b ) {
	return b ? BOOL_TRUE : BOOL_FALSE;
}

//---------------------------------------------------------------------------
// See if string is prefixed or suffixed by a string
//---------------------------------------------------------------------------

bool IsPrefixed( const string & s, const string & pre ) {
	return s.compare( 0, pre.size(), pre ) == 0;
}

bool IsSuffixed( const string & s, const string & suf ) {
	STRSIZE ssize = s.size();
	STRSIZE sufsize = suf.size();
	if ( ssize >= sufsize ) {
		return s.compare( ssize - sufsize, sufsize, suf ) == 0;
	}
	else {
		return false;
	}
}

//---------------------------------------------------------------------------
// Does the string contain a valid boolean?
//---------------------------------------------------------------------------

bool IsBool( const string & s ) {
	return s == BOOL_TRUE || s == BOOL_FALSE;
}

//------------------------------------------------------------------------
// Return string based on 's' with 'count' occurrences of 'target'
// replaced with the string 'with' - note 'target' is NOT a regexp
//------------------------------------------------------------------------

string Replace( const string & s,
				const string & target,
				const string & with,
				unsigned int count ) {
	STRPOS pos = 0;
	STRSIZE n = s.size(), t = target.size();
	string rv;
	while( count && pos < n ) {
		if ( s.substr( pos, t ) == target ) {
			rv += with;
			pos += t;
			count--;
		}
		else {
			rv += s[pos++];
		}
	}
	return rv;
}

//---------------------------------------------------------------------------
// Format using printf() format string. Validity of format is not checked.
// This uses the windows specific vsnprintf() function.
//---------------------------------------------------------------------------

string Format( const char * fmt, ... ) {
	const int BUFSIZE = 1024;
	int size = BUFSIZE, rv = -1;
	vector <char> buf;
	do {
		buf.resize( size );
		va_list valist;
		va_start(valist, fmt );
		// if vsnprintf() returns < 0, the buffer wasn't big enough
		// so increase buffer size and try again
		#ifdef WINVER
		rv = _vsnprintf( &buf[0], size, fmt, valist );
		#else
		rv = vsnprintf( &buf[0], size, fmt, valist );
		#endif
		va_end( valist );
		size *= 2;
	}
	while( rv < 0 );
	return string( &buf[0] );
}

//---------------------------------------------------------------------------
// Tokenise strings using separator 'sep'.  Tokens are appended to 'tok'
// and are trimmed using Trim(). Returns number of tokens extracted,
// which may be zero,
//---------------------------------------------------------------------------

unsigned int Split( const string & s, char sep, vector <string> & tok ) {

	STRPOS pos = 0;
	STRSIZE n = s.size();
	string t;

	while ( pos < n ) {
		char c = s[pos++];
		if ( c == sep ) {
			tok.push_back( Trim( t ) );
			t = EMPTYSTR;
		}
		else {
			t += c;
		}
	}

	if ( Not( IsEmpty( t ) && tok.size() == 0 ) ) {
		tok.push_back( Trim( t ) );
	}

	return tok.size();
}

//----------------------------------------------------------------------------
// As above but split on string of characters.
//----------------------------------------------------------------------------

unsigned int Split( const string & s, const string & sep,
						vector <string> & tok ) {

	STRPOS pos = 0;
	STRSIZE n = s.size();
	STRSIZE sn = sep.size();
	string t;

	if ( sep == "" ) {
		ATHROW( "Invalid separator in ALib::Split");
	}

	while ( pos < n ) {

		if ( PeekStr( s, pos, sn ) == sep ) {
			tok.push_back( t );
			t = "";
			pos += sn;
		}
		else {
			t += s[pos++];
		}
	}


	if ( Not( IsEmpty( t ) && tok.size() == 0 ) ) {
		tok.push_back( t );
	}

	return tok.size();
}

//---------------------------------------------------------------------------
// Tokenise string inro space-separated words, returnng number of words
// extracted, which may be zero.
//---------------------------------------------------------------------------

unsigned int Words( const string & s, vector <string> & words ) {
	bool in_word = false;
	STRPOS pos = 0;
	STRSIZE n = s.size();
	string w;
	while( pos < n ) {
		char c = s[pos++];
		if ( isspace( c ) ) {
			if ( in_word ) {
				words.push_back( w );
				w = EMPTYSTR;
			}
			in_word = false;
		}
		else {
			w += c;
			in_word = true;
		}
	}

	if ( w.size() ) {
		words.push_back( w );
	}

	return words.size();

}

//---------------------------------------------------------------------------
// Compare two strings, possibly ignoring case. Returns same values as
// strxmp() would have done.
//---------------------------------------------------------------------------

int Cmp( const string & s1, const string & s2, CaseSensitive cs ) {
	if ( cs == IgnoreCase ) {
		#ifdef WINVER
		return stricmp( s1.c_str(), s2.c_str() );
		#else
		return strcasecmp( s1.c_str(), s2.c_str() );
		#endif
	}
	else {
		return strcmp( s1.c_str(), s2.c_str() );
	}
}

//---------------------------------------------------------------------------
// Search list of null terminated strings (like argv) returning the string
// index or -1 if not found.
//---------------------------------------------------------------------------

int Find( const char * list[], const std::string & val ) {
	unsigned int i = 0;
	while( list[i] != 0 ) {
		if ( val == list[i] ) {
			return i;
		}
		i++;
	}
	return -1;
}

//---------------------------------------------------------------------------
// Search lists of strings in various forms
//---------------------------------------------------------------------------

bool In( const string & s, const string & commalist, CaseSensitive cs  ) {
	vector <string> values;
	Split( commalist, ',', values );
	return In( s, values, cs );
}

bool In( const string & s, const vector <string> & list, CaseSensitive cs ) {
	unsigned int n = list.size();
	for ( unsigned int i = 0; i < n; i++ ) {
		if ( Cmp( s, list[i], cs ) == 0 ) {
			return true;
		}
	}
	return false;
}

bool In( const string & s, CaseSensitive cs, const char * list, ... ) {
	const char * val = list;
	va_list begin;
	va_start( begin, list );
	while( val != NULL ) {
		if ( Cmp( s, val, cs ) == 0 ) {
			va_end( begin );
			return true;
		}
		val = va_arg( begin, const char * );
	}
	va_end( begin );
	return false;
}

//---------------------------------------------------------------------------
// Class wrapper for comma-separated lists
//---------------------------------------------------------------------------

CommaList :: CommaList( const string & s ) {
	Set( s );
}

unsigned int CommaList :: Set( const string & s ) {
	mItems.clear();
	return s == "" ?  0 : Split( s, ',', mItems );
}

unsigned int CommaList :: Size() const {
	return mItems.size();
}

const string & CommaList :: At( unsigned int i ) const {
	if ( i >= Size() ) {
		ATHROW( "CommaList index " << SQuote( Str(i) ) << " out of range" );
	}
	return mItems[i];
}

const vector <string> & CommaList :: Items() const {
	return mItems;
}

const CommaList & CommaList :: Append ( const CommaList & cl ) {
	mItems += cl.mItems;
	return *this;
}

const CommaList & CommaList :: Append ( const string & s ) {
	mItems.push_back( s ) ;
	return *this;
}

bool CommaList :: Contains( const string & s ) const {
	return ALib::Contains( mItems, s );
}

int CommaList :: Index( const string & s ) const {
	for( unsigned int i = 0; i < mItems.size(); i++ ) {
		if ( s == mItems[i] ) {
			return i;
		}
	}
	return -1;
}


//---------------------------------------------------------------------------
// Character ranges like a-z
//---------------------------------------------------------------------------

const char CR_INVERT = '!';			// invert sense of range
const char CR_HYPHEN = '-';			// character to use to specify range

//---------------------------------------------------------------------------
// Create range from string spec. Empty stringas are allowed.
//---------------------------------------------------------------------------

CharRange :: CharRange( const string & rs ) {
	mInvert = false;
	unsigned int pos = 0;
	DoSpecial( rs, pos );
	while( pos < rs.size() ) {
		ProcessChar( rs, pos );
	}
}

//---------------------------------------------------------------------------
// Handle special '!' (invert) and '-' (literal hyphen) at start of string
//---------------------------------------------------------------------------

void CharRange :: DoSpecial( const string & rs, unsigned int & pos ) {
	if ( rs.empty() ) {
		return;
	}
	if ( pos == 0 && ! rs.empty() && rs[0] == CR_INVERT ) {
		mInvert = true;
		pos++;
	}
	if ( pos < rs.size() && rs[pos] == CR_HYPHEN ) {
		mCharFlags[ '-' ] = 1;
		pos++;
	}
}

//---------------------------------------------------------------------------
// process character(s), advancing position in string
//---------------------------------------------------------------------------

void CharRange :: ProcessChar( const string & rs, unsigned int & pos ) {
	if ( pos == rs.size() - 1 ) {
		SetChar( rs[pos++] );
	}
	else if ( rs[pos + 1] == CR_HYPHEN ) {
		MakeHyphenRange( rs, pos );
	}
	else {
		SetChar( rs[pos++] );
	}
}

//---------------------------------------------------------------------------
// Expand hypen range like a-z
//---------------------------------------------------------------------------

void CharRange :: MakeHyphenRange( const string & rs, unsigned int & pos ) {
	unsigned int begin = rs[pos];
	pos += 2;
	if ( pos >= rs.size() ) {
		SetChar( begin );
		SetChar( '-' );
	}
	else {
		unsigned int end = rs[pos++];
		if ( end <= begin ) {
			ATHROW( "Invalid CharRange range \"" << begin << "-" << end << "\"" );
		}
		while( begin <= end ) {
			SetChar( begin++ );
		}
	}
}

//---------------------------------------------------------------------------
// Mark char as in range. It is an error to mark same char twice
//---------------------------------------------------------------------------

void CharRange :: SetChar( unsigned int c ) {
	if ( mCharFlags[c] ) {
		ATHROW( "Character '" << c << "' already specifird in CharRange" );
	}
	mCharFlags[c] = 1;
}

//---------------------------------------------------------------------------
// Concert bit flags to printable representation
//---------------------------------------------------------------------------

string CharRange :: Str() const {
	string r;
	for ( unsigned int i = 1; i < mCharFlags.size(); i++ ) {
		if ( (mInvert == false && mCharFlags[i])
			|| (mInvert == true && ! mCharFlags[i]) ) {
			r += char(i);
		}
	}
	return r;
}

//---------------------------------------------------------------------------
// See if range contains character
//---------------------------------------------------------------------------

bool CharRange :: Contains( unsigned char c ) const {
	if ( (mInvert == false && mCharFlags[c])
			|| (mInvert == true && ! mCharFlags[c]) ) {
		return true;
	}
	else {
		return false;
	}
}

//----------------------------------------------------------------------------
// Convert integer to binary bit pattern with optional left padding
//----------------------------------------------------------------------------

std::string BinStr( unsigned int n, char pad  ) {
	std::string s;
	bool have1 = false;
	const unsigned int nbits = sizeof(unsigned int) * 8;
	for ( unsigned int i = 0; i < nbits; i++ ) {
		if (  n & 0x80000000 ) {
			s += '1';
			have1 = true;
		}
		else {
			if ( have1 ||  pad ) {
				s += '0';
			}
		}
		n <<= 1;
	}
	return s;
}
//----------------------------------------------------------------------------

}	// namespace

//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
#include <vector>
using namespace ALib;
using namespace std;

DEFSUITE( "a_str" );

DEFTEST( TrimTest ) {
	string s = "   123   ";
	string t1 = ALib::Trim( s );
	FAILNE( t1, "123" );
	s = "foobar";
	Trunc( s, 3 );
	FAILNE( s, "foo" );
}



DEFTEST( SplitStrTest ) {
	string s = "foo--bar--1234";
	vector <string> tmp;
	Split( s, "--", tmp );
	FAILNE( tmp.size(), 3 );
	FAILNE( tmp[1], "bar" );
}

DEFTEST( AddifTest ) {
	string s = "";
	AddIfEmpty( s, "xxx" );
	FAILNE( s, "xxx" );
	s = "";
	AddIfNotEmpty( s, "xxx" );
	FAILNE( s, "" );
	s = "foo";
	AddIfEmpty( s, "xxx" );
	FAILNE( s, "foo" );
	s = "foo";
	AddIfNotEmpty( s, "xxx" );
	FAILNE( s, "fooxxx" );

}

DEFTEST( FmtTest ) {
	string s = Format( "value is %d", 42 );
	FAILNE( s, "value is 42" );
}


DEFTEST( PeekTest ) {
	string s = "foobar";
	char c = Peek( s, 3 );
	FAILNE( c, 'b' );
	c = Peek( s, 6 );
	FAILNE( c, 0 );
	bool pk = PeekStr( s, 2, "oba");
	FAILNE( pk, true  )
	pk = PeekStr( s, 2, "obx" );
	FAILNE( pk, false );
}

DEFTEST( PadTest ) {
	string s = "foo";
	string p = LeftPad( s, 5, '-' );
	FAILNE( p, "--foo" );
	p = RightPad( s, 5, '-' );
	FAILNE( p, "foo--" );
	p = Centre( s, 5 );
	FAILNE( p, " foo " );
	p = Centre( s, 6 );
	FAILNE( p, " foo  " );
}

DEFTEST( PrefSufTest ) {
	string s = "foo bar";
	FAILIF( IsPrefixed( s, "xyz" ) );
	FAILIF( IsPrefixed( s, "fox" ) );
	FAILIF( ! IsPrefixed( s, "foo" ) );
	FAILIF( IsSuffixed( s, "xyz" ) );
	FAILIF( IsSuffixed( s, "xar" ) );
	FAILIF( ! IsSuffixed( s, "bar" ) );
}

DEFTEST( QuoteTest ) {
	string nq = "foo";
	string q = "'foo'";
	FAILIF( ! IsQuoted( q ) );
	FAILIF( IsQuoted( nq ) );
	FAILNE( UnQuote( q ), nq );
}

DEFTEST( CaseTest ) {
	string s0 = "Hello World";
	string s1 = "HELLO WORLD";
	string c1 = Capitals( s1 );
	FAILIF( c1 != s0 );
}

DEFTEST( BinStrTest ) {
	unsigned int n = 0x1234;
	string s = BinStr( n  );
	FAILIF( s != "1001000110100" );
	n = (unsigned int ) -2;
	s = BinStr( n  );
	FAILIF( s != "11111111111111111111111111111110" );
}

DEFTEST( StrToVecTest ) {
	string s = "foo\nbar\none two\n";
	vector <string> v;
	StrToVec( s, v );
	FAILIF( v.size() != 3 );
	FAILIF( v[1] != "bar" );
}

//----------------------------------------------------------------------------

#endif

// end

