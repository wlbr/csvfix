//---------------------------------------------------------------------------
// a_regex.cpp
//
// Simple regular expression recogniser.
//
// Basic ideas and encoding scheme taken from "Software Tools In Pascal"
// by Kernighan & Plauger, but have been heavily re-worked to make them
// (I hope) more C++ friendly.
//
// ??? needs re-factoring after adding saved pattern matches ???
//
// Copyright (C) 2006 Neil Butterworth
//---------------------------------------------------------------------------

#include <iostream>
#include "a_base.h"
#include <bitset>
#include "a_except.h"
#include "a_regex.h"
#include "a_str.h"
#include "a_chsrc.h"

using std::string;
using std::vector;

//------------------------------------------------------------------------
// Begin ALib stuff
//------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// Regular expression special characters same as AWK except we don't do
// grouping or alternates.
//---------------------------------------------------------------------------

const char ANY_CHAR		= '.';	// match any single char

const char BEGIN_CHAR		= '^';	// start of string
const char END_CHAR		= '$';	// end of string

const char ZERO_MORE_CHAR	= '*';	// zero or more of preceding char
const char ONE_MORE_CHAR	= '+';	// one or more of preceding char
const char ZERO_ONE_CHAR	= '?';	// zero or one of preceding char

const char CC_INTRO_CHAR	= '[';	// character class introducer
const char CC_OUTRO_CHAR	= ']';	// character class terminator
const char CC_DASH_CHAR	= '-';	// dash in ranges e.g. a-z
const char CC_INVERT_CHAR	= '^';	// invert character class e.g. [^abc]

const char SPAT_INTRO		= '(';	// introduced saved pattern with '\('
const char SPAT_OUTRO		= ')';	// end saved pattern with '\)'

//---------------------------------------------------------------------------
// A Pos is returned when searching using a regex. It contains the
// start & length of a matched substring and a found/not found indicator.
// If the found indicator is not set, start and length are undefined.
//---------------------------------------------------------------------------

RegEx::Pos :: Pos()
	: mStart(0), mLen(0), mFound( false ) {
}

RegEx::Pos :: Pos( unsigned int start,
								unsigned int len,
								bool found )
	: mStart(start), mLen(len), mFound( found ) {
}

//----------------------------------------------------------------------------
// Static helper to escape all characters in string
//----------------------------------------------------------------------------

string RegEx :: Escape( const string & s ) {
	string r;
	for ( unsigned int i = 0; i < s.size(); i++ ) {
		r += '\\';
		r += s[i];
	}
	return r;
}

//------------------------------------------------------------------------
// Zero-based starting position for a match
//------------------------------------------------------------------------

unsigned int RegEx::Pos :: Start() const {
	return mStart;
}

//------------------------------------------------------------------------
// Length of matched characters
//------------------------------------------------------------------------

unsigned int RegEx::Pos :: Length() const {
	return mLen;
}

//------------------------------------------------------------------------
// Indicates if match was found
//------------------------------------------------------------------------

RegEx::Pos :: operator bool() const {
	return mFound;
}

bool RegEx::Pos :: Found() const {
	return mFound;
}

//---------------------------------------------------------------------------
// Bitmap used to represent char ranges, one bit for each character in a
// range. Only chars with ASCII codes 0 to 127 are supported.
//---------------------------------------------------------------------------

class RegEx::CharBitMap {

	public:

		CharBitMap( const std::string & s ) {
			for ( unsigned int i = 0; i < s.size(); i++ ) {
				Add( s[i] );
			}
		}

		void Add( unsigned char c ) {
			if ( c >= MAXSIZE ) {
				ATHROW( "Character value " << int(c) << " too big" );
			}
			mBits.set( c );
		}

		bool Contains( unsigned char c ) const {
			if ( c >= MAXSIZE ) {
				ATHROW( "Character value " << int(c) << " too big" );
			}
			return mBits.test( c );
		}

		void Clear() {
			mBits.reset();
		}

		void Invert() {
			mBits.flip();
		}

		string ToString() const {
			string s;
			for ( unsigned char i = 0; i < MAXSIZE; i++ ) {
				if ( Contains( i ) ) {
					if ( i < ' ' ) {
						s += '.';
					}
					else {
						s += i;
					}
				}
			}
			return s;
		}

	private:

		enum { MAXSIZE = 128 };
		std::bitset <MAXSIZE> mBits;
};

//---------------------------------------------------------------------------
// Each character in a regexp is encoded before matching so that the matching
// code doesn't need to bother about syntax checking etc. Each encoding
// entry has a type indixator and a bitmap indicating which characters
// the encoding will match against.
//---------------------------------------------------------------------------

class RegEx::Encoding {

	public:

		// create encoded regexp from string representation
		Encoding( const string & expr, bool csense = true );

		// get string rep for debug
		string ToString() const;

		// how many entries
		unsigned int Size() const {
			return mEntries.size();
		}

		// classification methods
		bool IsClosure( unsigned int i ) const {
			return mEntries[i].Type() == Entry::Close;
		}

		bool IsZeroOne( unsigned int i ) const {
			return mEntries[i].Type() == Entry::ZeroOne;
		}

		bool IsBegin( unsigned int i ) const {
			return mEntries[i].Type() == Entry::Begin;
		}

		bool IsEnd( unsigned int i ) const {
			return mEntries[i].Type() == Entry::End;
		}

		bool IsChar( unsigned int i ) const {
			return mEntries[i].Type() == Entry::Char;
		}

		bool Match( unsigned char c, unsigned int i ) const;

		void ClearAllSaved();
		unsigned int SavedCount() const;
		std::string SavedAt( unsigned int i ) const;
		void ClearSavedPat( unsigned int  ei, unsigned int len  ) const;
		unsigned int GetSavedPatLen( unsigned int ei ) const;

	private:

		bool EncodeClosure( char c );
		void EncodeCharClass( CharSource & src );
		void CheckPrevChar( int i ) const;
		void MakeLastEntryCaseInsense();
		void SetLastEntryTag();

		void AddToSavedPat( unsigned char c, unsigned int ei ) const;

		class Entry {

			public:
				enum EType { None, Char, Close, ZeroOne, Begin, End };

				Entry( EType t = None,
						const std::string & chars = "",
						bool invert = true )
						: mType( t ), mMap( chars ), mTag(0) {
					if ( ! invert ) {
						mMap.Invert();
					}
				}

				EType Type() const { return mType; }

				void MakeCaseInsense() {
					for( unsigned int i = 0; i < 26; i++ ) {
						if ( mMap.Contains( 'A' + i ) ) {
							mMap.Add( 'a' + i );
						}
						if ( mMap.Contains( 'a' + i ) ) {
							mMap.Add( 'A' + i );
						}
					}
				}

				bool Match( unsigned char c ) const {
					return mMap.Contains( c );
				}

				// used to convert char to closure#ifndef
				void ChangeType( EType t ) {
					mType = t;
				}

				// only used for debugging display
				string ToString() const {
					string rv;
					switch( mType ) {
						case Begin:		rv += "BEGIN    "; break;
						case End:		rv += "END      "; break;
						case Char:		rv += "CHAR     "; break;
						case Close:		rv += "CLOSE(*) "; break;
						case ZeroOne:	rv += "CLOSE(?) "; break;
						default:		rv += "????  " 	+ Str( int(mType) );
					}
					return rv + mMap.ToString();
				}


				void SetTag( unsigned int t ) {
					mTag = t;
				}

				unsigned int Tag() const {
					return mTag;
				}

			private:
				EType mType;
				RegEx::CharBitMap mMap;
				unsigned int mTag;
		};

		std::vector <Entry> mEntries;
		unsigned int mNextTag;
		bool mInSavedPat;
		mutable std::vector <string> mSaved;

};

//---------------------------------------------------------------------------
// Make an encoding entry be case-insensitive
//---------------------------------------------------------------------------

void RegEx::Encoding :: MakeLastEntryCaseInsense() {
	Entry & e = mEntries[ mEntries.size() - 1 ];
	e.MakeCaseInsense();
}

//----------------------------------------------------------------------------
// Set the saved pattern tag for the last encoding entry
//----------------------------------------------------------------------------

void RegEx::Encoding :: SetLastEntryTag() {
	Entry & e = mEntries[ mEntries.size() - 1 ];
	e.SetTag( mInSavedPat ? mNextTag : 0 );
}

//---------------------------------------------------------------------------
// Helper to check previous encoding entry is not a closure
//---------------------------------------------------------------------------

void RegEx::Encoding :: CheckPrevChar( int i ) const {
	if ( i < 0 || ! IsChar( i ) ) {
		ATHROW( "Invalid closure" );
	}
}

//----------------------------------------------------------------------------
// Add character c to saved pattern indicated by the tag of the encoding
// entry indexed by ei. If tag is zero, this encoding is not part of a
// saved pattern, so discard the character.
//----------------------------------------------------------------------------

void RegEx::Encoding ::AddToSavedPat( unsigned char c,
											unsigned int ei ) const {
	unsigned int i = mEntries.at(ei).Tag();
	if ( i ) {
		mSaved.at(i - 1) += c;
	}
}

//----------------------------------------------------------------------------
// Clear the saved patter referenced by the tag of encoding entre ei down to
// length specified by len.
//----------------------------------------------------------------------------

void RegEx::Encoding :: ClearSavedPat( unsigned int  ei,
										unsigned int len ) const {
	unsigned int i = mEntries.at(ei).Tag();
	if ( i ) {
		mSaved.at(i - 1).resize( len );
	}
}

//----------------------------------------------------------------------------
// Reset all saved pattern matches.
//----------------------------------------------------------------------------

void RegEx::Encoding :: ClearAllSaved() {
	for ( unsigned int i = 0; i < mSaved.size(); i++ ) {
		mSaved.at(i).clear();
	}
}

//----------------------------------------------------------------------------
// How many saved matches?
//----------------------------------------------------------------------------

unsigned int RegEx::Encoding :: SavedCount() const {
	return mSaved.size();
}

//----------------------------------------------------------------------------
// Get saved match at index. Note that index is zero based but in replacement
// expressions and tags it will be 1-based.
//----------------------------------------------------------------------------

string RegEx::Encoding :: SavedAt( unsigned int i ) const {
	return mSaved.at(i);
}

//----------------------------------------------------------------------------
// Get length of a saved pattern referenced by encoding entries tag
//----------------------------------------------------------------------------

unsigned int RegEx::Encoding :: GetSavedPatLen( unsigned int ei ) const {
	unsigned int i = mEntries.at(ei).Tag();
	if ( i ) {
		return mSaved.at( i - 1 ).size();
	}
	else {
		return 0;
	}
}
//----------------------------------------------------------------------------
// see if regexp character indexed by 'i' can match target char 'c'. If it
// does, add the matched character to the relevant saved pattern (if any).
//----------------------------------------------------------------------------

bool RegEx::Encoding :: Match( unsigned char c, unsigned int i ) const {
	if ( i >= Size() ) {
		ATHROW( "Invalid Encoding index: " << i );
	}
	bool matched =  mEntries[i].Match( c );
	if ( matched ) {
		AddToSavedPat( c, i );
	}
	return matched;
}

//---------------------------------------------------------------------------
// Create a closure. We support "zero or more", "one or more" and "zero or
// one" closures. The "one or more" closure is implemented using a "char"
// followed by a "zero or more" closure. Closures must be preceded by a
// non-closure e.g. "a**" is an error
//
// Returns true if we encoded a closure, false otherwise.
//---------------------------------------------------------------------------

bool RegEx::Encoding :: EncodeClosure( char c ) {

	int prev = mEntries.size() - 1;		// index of previous entry
	bool rv = true;

	if ( c == ZERO_MORE_CHAR ) {
		CheckPrevChar( prev );
		mEntries[prev].ChangeType( Entry::Close );
	}
	else if ( c == ONE_MORE_CHAR ) {
		CheckPrevChar( prev );
		Entry e = mEntries[ prev ];
		e.ChangeType( Entry::Close );
		mEntries.push_back( e );
	}
	else if ( c == ZERO_ONE_CHAR ) {
		CheckPrevChar( prev );
		mEntries[ prev ].ChangeType( Entry::ZeroOne );
	}
	else {
		rv = false;		// not a closure
	}
	return rv;
}


//---------------------------------------------------------------------------
// Helper to create a range e.g. a-d is converted to "abcd"
//---------------------------------------------------------------------------

static string MakeRange( char c1, char c2 ) {
	if ( c1 >= c2 ) {
		ATHROW( "Invalid range: " << c1 << "-" << c2 );
	}
	string rv;
	while( c1 <= c2 ) {
		rv += c1++;
	}
	return rv;
}

//---------------------------------------------------------------------------
// Encode a character class. Character classes are constructs like [a-z].
// They can be inverted e.g. [^a-z]  matches any non-lowercase character.
//---------------------------------------------------------------------------

void RegEx::Encoding :: EncodeCharClass( CharSource & src ) {

	CharSource::Char c;

	// handle any character set inversion (!) character
	// subsequent '!' chars will be treated literally - this is intentional
	bool invert = false;
	if ( src.Peek( c ) && c.Value() == CC_INVERT_CHAR ) {
		invert = true;
		src.Next( c );
	}

	string s;
	while( src.Next( c ) ) {
		if ( c.Value() == CC_OUTRO_CHAR && ! c.Escaped() ) {
			if ( s == "" ) {
				ATHROW( "Empty character class" );
			}
			Entry e = Entry( Entry::Char, s, ! invert );
			mEntries.push_back( e );
			return;
		}
		else if ( c.Value() != CC_DASH_CHAR || s.size() == 0 ) {
			s += c.Value();
		}
		else if ( c.Value() == CC_DASH_CHAR ) {
			if ( ! src.Next( c ) ) {
				ATHROW( "Invalid character class" );
			}
			s += MakeRange( s[s.size()-1] + 1, c.Value() );
		}
	}
	ATHROW( "Unterminated character class" );
}

//---------------------------------------------------------------------------
// Constructor creates an encoded regular expression from a string.
// Now deal with saved patterns in the form \(pat\).
//---------------------------------------------------------------------------

RegEx::Encoding :: Encoding( const string & expr, bool csense )
					: mNextTag( 1 ), mInSavedPat( false ) {

	CharSource src( expr );
	CharSource::Char c;

	while( src.Next( c ) ) {
		if ( c.Escaped() ) {
			if ( c.Value() == SPAT_INTRO ) {
				if ( mInSavedPat ) {
					ATHROW( "Invalid saved pattern nesting" );
				}
				mInSavedPat = true;
				mSaved.push_back( "" );
				continue;
			}
			else if ( c.Value() == SPAT_OUTRO ) {
				if ( ! mInSavedPat ) {
					ATHROW( "Invalid saved pattern nesting" );
				}
				mInSavedPat = false;
				mNextTag++;
				continue;
			}
			else {
				mEntries.push_back( Entry( Entry::Char, Str( c.Value() ) ) );
			}
		}
		else if ( c.Value() == BEGIN_CHAR && mEntries.size() == 0 ) {
			mEntries.push_back( Entry( Entry::Begin, "" ) );
		}
		else if ( c.Value() == END_CHAR && src.AtEnd() ) {
			mEntries.push_back( Entry( Entry::End, "" ) );
		}
		else if ( c.Value() == ANY_CHAR ) {
			mEntries.push_back( Entry( Entry::Char, "", false ) );
		}
		else if ( EncodeClosure( c.Value() ) ) {
			// nothing to do
		}
		else if ( c.Value() == CC_INTRO_CHAR ) {
			EncodeCharClass( src );
		}
		else {	// ordinary character or spcial in non-special Pos
			mEntries.push_back(  Entry( Entry::Char, Str( c.Value() ) ) );
		}

		if ( ! csense ) {
			MakeLastEntryCaseInsense();
		}

		SetLastEntryTag();

	}
}

//---------------------------------------------------------------------------
// Convert encoding to string representation for debugging
//---------------------------------------------------------------------------

string RegEx::Encoding :: ToString() const {
	string rv;
	for( unsigned int i = 0; i < mEntries.size(); i++ ) {
		rv += mEntries[i].ToString() + "\n";
	}
	rv += "\n";
	return rv;
}


//---------------------------------------------------------------------------
// Create regex from string
//---------------------------------------------------------------------------

RegEx :: RegEx( const string & expr, CaseSense csense  )
	: mEnc( 0 ) {
	mEnc = new Encoding( expr, csense == Sensitive );
}

//---------------------------------------------------------------------------
// Copy/assign
//---------------------------------------------------------------------------

RegEx :: RegEx( const RegEx & ex ) {
	mEnc = new Encoding( * ex.mEnc );
}

void RegEx :: operator =( const RegEx & ex ) {
	delete mEnc;
	mEnc = new Encoding( * ex.mEnc );
}


//---------------------------------------------------------------------------
// Trivial dtor
//---------------------------------------------------------------------------

RegEx :: ~RegEx() {
	Clear();
}


//----------------------------------------------------------------------------
// Get saved expression match value
//----------------------------------------------------------------------------

string RegEx :: SavedMatch( unsigned int i ) const {
	if ( i >= mEnc->SavedCount() ) {
		ATHROW( "No such saved expression value" );
	}
	return mEnc->SavedAt(i);
}

unsigned int RegEx :: SavedMatchCount() const {
	return mEnc->SavedCount();
}

//---------------------------------------------------------------------------
// Check if the character s[sindex] matches an encoded regexp character
//---------------------------------------------------------------------------

bool RegEx :: MatchChar( const std::string & s,
							 unsigned int sindex,
							 unsigned int eindex ) const {

	return mEnc->Match( s[sindex], eindex );
}

//---------------------------------------------------------------------------
// Main user-callable function. See if a regexp can be found in a string
// starting at Pos start. Returns a Pos object containing
// success indicator & start/length of found substring
//---------------------------------------------------------------------------

RegEx::Pos RegEx :: FindIn( const string & s, unsigned int start ) const {

	if ( s == "" ) {
		return MatchEmpty();
	}

	unsigned int ssize = s.size();
	if ( start >= ssize ) {
		ATHROW( "String start out of range: " << start );
	}

	mEnc->ClearAllSaved();
	Pos pos;


	for ( unsigned int i = start; i < ssize; i++ ) {
		pos = MatchAt( s, i, 0 );
		if ( pos.Found() ) {
			return pos;
		}
	}

	return pos;
}

//----------------------------------------------------------------------------
// Quick hack to deal with empty string matching problems.
//----------------------------------------------------------------------------

RegEx::Pos RegEx :: MatchEmpty() const {
	const RegEx::Pos FOUND( 0, 0, true );
	const RegEx::Pos NOTFOUND( 0, 0, false );

	for ( unsigned int  i = 0; i < mEnc->Size(); i++ ) {
		if ( mEnc->IsChar( i ) ) {
			return NOTFOUND;
		}
	}
	return FOUND;
}

//---------------------------------------------------------------------------
// Replace single occurrence of v in i s at start, returning a pos which
// contains success indicator.
//---------------------------------------------------------------------------

RegEx::Pos RegEx :: ReplaceIn( string & s, const string & v,
									unsigned int start ) {
	Pos pos = FindIn( s, start );
	if ( pos.Found() ) {
		string t = ReplaceSaved( v );
		s = s.substr( 0, start )
			+ s.substr( start, pos.Start() - start )
			+ t
			+ s.substr( pos.Start() + pos.Length() );
	}
	return pos;
}

//----------------------------------------------------------------------------
// Replaved saved pattern speclai values like \1, \2 etc. with saved values
//----------------------------------------------------------------------------

string RegEx :: ReplaceSaved( const string & s ) const {

	string r;
	unsigned int pos = 0;

	while( char c = Peek( s, pos ) ) {
		if ( c == '\\' ) {
			pos ++;
			char nc = Peek( s, pos );
			if ( nc >= '1' && nc <= '9' ) {
				unsigned int  n = nc - '1';
				if ( SavedMatchCount() <=  n ) {
					ATHROW( "Invalid saved pattern marker: \\" << nc );
				}
				r += SavedMatch( n );
			}
			else {
				r += nc;
			}
		}
		else {
			r += c;
		}
		pos++;
	}
	return r;
}

//---------------------------------------------------------------------------
// Replace all occurrences of s in v, returning number of replcements
//---------------------------------------------------------------------------

unsigned int RegEx :: ReplaceAllIn( string & s, const string & v ) {

	unsigned int rcount = 0, start = 0;

	while( start < s.size() ) {

		Pos pos = FindIn( s, start );
		if ( ! pos.Found() ) {
			break;
		}

		string t = ReplaceSaved( v );
		s = s.substr( 0, start )
			+ s.substr( start, pos.Start() - start )
			+ t
			+ s.substr( pos.Start() + pos.Length() );
		start = pos.Start() +  t.size();
		rcount++;
	}

	return rcount;
}

//---------------------------------------------------------------------------
// Match a maximum of 'maxlen' characters of the string 's' starting st
// index 'si' against the encoded closure indexed by 'eindex'.
// Returns actual number of characters that were matched.
//---------------------------------------------------------------------------

unsigned int RegEx :: MatchClosure( const std::string & s,
										unsigned int si,
										unsigned int eindex,
										unsigned int maxlen ) const {
	unsigned int n = 0, sz = s.size();
	while( n < maxlen && si + n < sz && MatchChar( s, si + n, eindex ) ) {
		n++;
	}
	return n;
}

//---------------------------------------------------------------------------
// Recursive routine that does all the real work. The basic idea is that
// if we have a closure (encoding indexed by 'eindex'), we try to match
// against the longest possible substring and then try to match the rest
// of the string (recursively) aginst the rest of the regex. Non-closure
// regex characters are handled iteratively.
//
// If matching fails, we reduce the length of the closure match by one
// amd try again. If the length of the closure match falls to zero & we still
// can't match the remainder, we have failed.
//
// we now also have to deal with saved patterns, shortening the saved match
// if a closure match fails and and the closure length is reduced.
//---------------------------------------------------------------------------

RegEx::Pos RegEx :: MatchAt( const string & s,
								unsigned int start,
								unsigned int eindex )	const {

	const Pos NOTFOUND;					// "search failed" return value
	const unsigned int SZ = s.size();	// search string size

	unsigned int matchlen = UINT_MAX;	// big closure length to start with
	unsigned int sindex = 0;			// place we are at in searched string

	while( start + sindex < SZ && eindex < mEnc->Size() ) {

		unsigned int spatlen = mEnc->GetSavedPatLen( eindex );

		if ( mEnc->IsClosure( eindex ) ) {			// zero/many closure
			matchlen = MatchClosure( s, start + sindex, eindex, matchlen );
			// recursively try to match everything following closure
			Pos pos = MatchAt( s, start + sindex + matchlen, eindex + 1 );
			if ( pos.Found() ) {
				return Pos( start, sindex + matchlen + pos.Length(), true );
			}
			else if ( matchlen > 0 ) {
				mEnc->ClearSavedPat( eindex, spatlen );
				matchlen--;			// failed - reduce length of closure match
				continue;			// and try again
			}
			else {
				return NOTFOUND;	// failed - unwind recursion
			}
		}
		else if ( mEnc->IsBegin( eindex ) ) {	// begin string
			if ( start + sindex == 0 ) {
				eindex++;
			}
			else {
				return NOTFOUND;
			}
		}
		else if ( mEnc->IsZeroOne( eindex ) ) {	// zero/one closure
			if ( MatchChar( s, start + sindex, eindex ) ) {
				sindex++;
			}
			eindex++;
		}
		else if ( MatchChar( s, start + sindex, eindex ) ) {	// single char
			sindex++;
			eindex++;
		}
		else {
			return NOTFOUND;
		}
	}

	// if we have walked over all encoded regexp characters except
	// for the end($) marker (if there is one) then success!
	if ((start + sindex == SZ
			&& eindex < mEnc->Size()
			&& mEnc->IsEnd( eindex )
		)
		|| eindex == mEnc->Size() ) {
		return Pos( start, sindex, true );
	}
	else {
		return NOTFOUND;
	}
}

//---------------------------------------------------------------------------
// Ditches any encoding
//---------------------------------------------------------------------------

void RegEx :: Clear() {
	delete mEnc;
	mEnc = 0;
}

//---------------------------------------------------------------------------
// Return string rep of encoding for debug etc.
//---------------------------------------------------------------------------

string RegEx :: GetEncoding() const {
	if ( mEnc == 0 ) {
		return "Empty regex";
	}
	else {
		return mEnc->ToString();
	}
}

//------------------------------------------------------------------------

}  // namespace

//----------------------------------------------------------------------------
// tests
//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_regex" );

DEFTEST( Regex1 ) {
	string s = "aaa1234zzz";
	RegEx re( "aaa[0-9].*zzz");
	RegEx::Pos p = re.FindIn( s );
	FAILNE( p.Found(), true );
}

DEFTEST( Saved1 ) {
	string s = "aaa1234zzz";
	RegEx re( "aaa\\([0-9].*\\)zzz");
	RegEx::Pos p = re.FindIn( s );
	FAILNE( p.Found(), true );
	FAILNE( re.SavedMatch(0), "1234" );
}

struct RETestData {
	const char * str;
	const char * expr;
	int pos;
};

static RETestData TD[] = {
	{ "aaa", "aaa", 0 },
	{ "aaa1234zzz", "aaa[0-9]*zzz", 0 },
	{ "aaa1234zzz", "1234", 3 },
	{ "aaa1234zzz", ".$", 9 },
	{ "aaa1234zzz", "a[0-9].*", 2 },
	{ 0, 0, 0 }
};

DEFTEST( Multi1 ) {
	unsigned int i = 0;
	while( TD[i].str ) {
		RegEx re( TD[i].expr );
		string s( TD[i].str );
		RegEx::Pos p = re.FindIn( s );
		if ( TD[i].pos == -1 ) {
			FAILNE( p.Found(), false );
		}
		else {
			FAILNE( p.Found(), true );
			FAILNE( p.Start(), (unsigned int) TD[i].pos );
		}
		i++;
	}
}

struct RESPData {
	const char * str;
	const char * expr;
	const char * saved;
};

static RESPData PD[] = {
	{ "aaa", "a\\(aa\\)", "aa" },
	{ "aaa1234zzz", "aaa\\([0-9]*\\)zzz", "1234" },
	{ "aaa1234zzz", "aaa\\([0-9].*\\)zzz", "1234" },
	{ 0, 0, 0 }
};

DEFTEST( Multi2 ) {
	unsigned int i = 0;
	while( PD[i].str ) {
		RegEx re( PD[i].expr );
		string s( PD[i].str );
		RegEx::Pos p = re.FindIn( s );
		FAILNE( p.Found(), true );
		FAILNE( re.SavedMatch(0), PD[i].saved );
		i++;
	}
}


DEFTEST( RepSaved ) {

	string s = "aaa1234zzz";
	RegEx re1( "1234" );
	re1.ReplaceIn( s, "XX", 0 );
	FAILNE( s, "aaaXXzzz" );

	s = "aaa1234zzz";
	RegEx re2( "\\([0-9][0-9]*\\)" );
	RegEx::Pos p = re2.FindIn( s );
	FAILNE( p.Found(), true );
	FAILNE( re2.SavedMatch(0), "1234" );
	re2.ReplaceIn( s, "X\\1X", 0 );
	FAILNE( s, "aaaX1234Xzzz" );

	s = "aaa1234zzz";
	p = re2.FindIn( s );
	FAILNE( p.Found(), true );
	FAILNE( re2.SavedMatch(0), "1234" );
	re2.ReplaceIn( s, "X\\\\1X", 0 );
	FAILNE( s, "aaaX\\1Xzzz" );

	s = "aaa1234zzz";
	p = re2.FindIn( s );
	FAILNE( p.Found(), true );
	FAILNE( re2.SavedMatch(0), "1234" );
	re2.ReplaceIn( s, "X\\1X\\1X", 0 );
	FAILNE( s, "aaaX1234X1234Xzzz" );

	s = "aaa123---XXXzzz";
	RegEx re4( "\\([0-9][0-9]*\\)---\\(XX*\\)z"  );
	p = re4.FindIn( s );
	FAILNE( p.Found(), true );
	FAILNE( re4.SavedMatch(0), "123" );
	FAILNE( re4.SavedMatch(1), "XXX" );
}


DEFTEST( RepAll ) {

	string s = "1x234x5y6";
	RegEx re1( "\\([xy]\\)" );
	RegEx::Pos p = re1.FindIn( s );
	FAILNE( p.Found(), true );
	re1.ReplaceAllIn( s, "Z\\1" );
	FAILNE( s, "1Zx234Zx5Zy6" );

	s = "aaa123---XXXzzz";
	RegEx re2( "\\([0-9][0-9]*\\)---\\(XX*\\)z"  );
	p = re2.FindIn( s );
	FAILNE( p.Found(), true );
	re2.ReplaceAllIn( s, "\\2\\1");
	FAILNE( s, "aaaXXX123zz" );
}

// tests for match against empty string, which the original design does
// not allow for :-(

DEFTEST( EmptyStr ) {
	RegEx re1( ".*" );
	string s = "";
	RegEx::Pos p = re1.FindIn( s );
	FAILNE( p.Found(), true );

	RegEx re2( "^$" );
	p = re2.FindIn( s );
	FAILNE( p.Found(), true );

	RegEx re3( "x" );
	p = re3.FindIn( s );
	FAILNE( p.Found(), false );
}

#endif

//----------------------------------------------------------------------------

// end
