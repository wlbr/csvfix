//---------------------------------------------------------------------------
// a_csv.cpp
//
// CSV handling for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include <iostream>
#include <fstream>
#include "a_except.h"
#include "a_csv.h"
#include "a_str.h"
using std::vector;
using std::string;

namespace ALib {

//---------------------------------------------------------------------------
// Do nothing ctor & dtor
//---------------------------------------------------------------------------

CSVLineParser :: CSVLineParser( char sep )
	: mSep( sep ) {
	if ( ! IsValidSep( sep ) ) {
		ATHROW( "Invalid CSV field separator: " << sep );
	}
}

CSVLineParser :: ~CSVLineParser() {
}

//---------------------------------------------------------------------------
// Parse a string containing a line of CSV data into a vector
//---------------------------------------------------------------------------

void CSVLineParser :: Parse( const string & csv, vector <string> & data ) {
	mPos = 0;
	mCSV = & csv;
	data.clear();
	mMore = true;
	while( mMore ) {
		mMore = false;
		char c = Peek();
		if ( c == '"' ) {
			data.push_back( GetQuoted() );
		}
		else {
			data.push_back( GetNonQuoted() );
		}
	}
}

//----------------------------------------------------------------------------
// Is character a valid field sepparator? Must not be alpha num or whitespace
// or double-quote.
//----------------------------------------------------------------------------

bool CSVLineParser :: IsValidSep( char c ) {
	return ! isalnum( c ) && ! isspace( c ) && c != '"';
}

//---------------------------------------------------------------------------
// Read string in double-quotes. Handles use of "" to embed a quote
//---------------------------------------------------------------------------

string CSVLineParser :: GetQuoted() {
	string field;
	Next();
	while( char c = Peek() ) {
		if ( c == '"' ) {
			Next();
			c = Peek();
			if ( c == '"' ) {
				field += c;
				Next();
			}
			else {
				if ( Peek() == mSep ) {
					Next();
					mMore = true;
					return field;
				}
				break;
			}
		}
		else {
			field += c;
			Next();
		}
	}
	mMore = Peek() != 0;
	return field;
}

//---------------------------------------------------------------------------
// Get non-quoted string which will be terminated by comma or EOS
//---------------------------------------------------------------------------

string CSVLineParser :: GetNonQuoted() {
	string field;
	char c;
	while( (c = Peek()) ) {
		if ( c == mSep ) {
			mMore = true;
			Next();
			break;
		}
		field += c;
		Next();
	}
	return field;
}

//---------------------------------------------------------------------------
// Get character from input, or 0 if no more
//---------------------------------------------------------------------------

char CSVLineParser :: Peek() const {
	if ( mPos >= mCSV->size() ) {
		return 0;
	}
	else {
		return mCSV->at( mPos );
	}
}

//---------------------------------------------------------------------------
// Get character from input, advancing scan
//---------------------------------------------------------------------------

char CSVLineParser :: Next() {
	char c = Peek();
	if ( c == 0 ) {
		return 0;
	}
	else {
		mPos++;
	}
	return c;
}

//---------------------------------------------------------------------------
// Base class for file/stream parses
//---------------------------------------------------------------------------

CSVParser :: CSVParser( char sep )
	: mLineParser ( sep ) {
}

CSVParser :: ~CSVParser() {
	// nothing
}


//---------------------------------------------------------------------------
// Parse from istream. Parser does not own the stream.
//---------------------------------------------------------------------------

CSVStreamParser :: CSVStreamParser( std::istream & is,
										bool igblank, bool skipcols,
										bool colmap,
										char csvsep  )
	: CSVParser( csvsep ), mStream( & is ), mLineNo( 0 ),
		mIgnoreBlankLines( igblank ),
		mSkipColumnNames( skipcols ), mMakeColMap( colmap )  {
}

//---------------------------------------------------------------------------
// Line number in stream
//---------------------------------------------------------------------------

unsigned int CSVStreamParser :: LineNo() const {
	return mLineNo;
}

//---------------------------------------------------------------------------
// Raw input
//---------------------------------------------------------------------------

string CSVStreamParser :: RawLine() const {
	return mRawLine;
}

//---------------------------------------------------------------------------
// Parse next line of input from stream, returning false at end of input
// or when stream goes badi. We read single character at time and check
// if it is data or a termibator using the ProcessChar state machine. Then
// parse the whole line (possibly including newlines) that we have
// accumulated using the normal line parser.
//---------------------------------------------------------------------------

bool CSVStreamParser :: ParseNext( vector <string> & data ) {

	string line;
	char c;
	mState = OutVal;

	while(  mStream->get( c ) ) {

		if ( c == '\n' ) {			// new line in source
			mLineNo++;
		}
		else if ( c == '\r' ) {	// may or may not be CRs
			continue;				// in any case, we don't want them
		}


		if ( ! ProcessChar( c, line ) ) {   // end of line of data
			if ( mMakeColMap && mLineNo == 1 ) {
				MakeColMap( line );
			}
			if ( (mIgnoreBlankLines && ALib::IsEmpty( line ))
					|| (mSkipColumnNames && mLineNo == 1) ) {
				line = "";
				mState = OutVal;
				continue;
			}
			break;
		}
	}

	if ( line == "" && mStream->eof() ) {
		return false;
	}

	mRawLine = line;
	LineParser().Parse( line, data );

	return true;
}

//------------------------------------------------------------------------
// Get zero-based column index from name in column map.
//------------------------------------------------------------------------

unsigned int CSVStreamParser :: ColIndexFromName(const string & name ) const {
	if ( mColMap.size() == 0 ) {
		ATHROW( "CSV parser has no column map" );
	}
	ColNameMapType::const_iterator it = mColMap.find( name );
	if ( it == mColMap.end() ) {
		ATHROW( "Unknown column name: " << name );
	}
	return it->second;
}

//------------------------------------------------------------------------
// Create map of column names to zero-based coumn indexes
//------------------------------------------------------------------------

void CSVStreamParser :: MakeColMap( const std::string & cols ) {

	if ( ALib::IsEmpty( cols ) ) {
		ATHROW( "No column names available" );
	}

	mColMap.clear();
	vector <string> data;
	LineParser().Parse( cols, data );

	for ( unsigned int i = 0; i < data.size(); i++ ) {
		if ( mColMap.find( data[i]  ) != mColMap.end() ) {
			ATHROW( "Duplicate column name " << data[i] );
		}
		mColMap.insert( std::make_pair( data[i], i ) );
	}
}

//---------------------------------------------------------------------------
// This state machine is used to determine whether or not a newline
// should be considered part of the data or as a terminator. It returns
// true if the newline is data, false if line terminator.
//---------------------------------------------------------------------------

bool CSVStreamParser :: ProcessChar( char c, string & line ) {

	if ( mState == OutVal ) {
		if ( c == LineParser().Separator() ) {
			mState = OutVal;
		}
		else {
			switch( c ) {
				case '"':	mState = InQVal; break;
				case '\n':	return false;
				default:	mState = InVal;
			}
		}
	}
	else if ( mState == InVal ) {
		if ( c == LineParser().Separator() ) {
			mState = OutVal;
		}
		else if ( c == '\n' ) {
			return false;
		}
		else {
			mState = InVal;
		}
	}
	else if ( mState == InQVal ) {
		switch( c ) {
			case '"':	mState = HaveQ; break;
			default:	mState = InQVal;
		}
	}
	else if ( mState == HaveQ ) {
		switch( c ) {
			case '"':	mState = InQVal; break;
			case '\n':	return false;
			default:	mState = OutVal;
		}
	}

	line += c;

	return true;

}

//---------------------------------------------------------------------------
// Parse named file - manages the created input file stream
//---------------------------------------------------------------------------

CSVFileParser :: CSVFileParser( const string & fname, bool igblank,
								char csvsep  )
	: mIfstream( 0 ), mParser( 0  ), mFileName( fname ) {

	mIfstream = new std::ifstream( fname.c_str() );
	if ( ! mIfstream->is_open() ) {
		delete mIfstream;
		mIfstream = 0;
		ATHROW( "Cannot open CSV file " << fname << " for input" );
	}
	mParser = new CSVStreamParser( * mIfstream, igblank, csvsep );
}

//---------------------------------------------------------------------------
// Free input stream & parser
//---------------------------------------------------------------------------

CSVFileParser :: ~CSVFileParser() {
	delete mParser;
	delete mIfstream;
}

//---------------------------------------------------------------------------
// Forward to stream parser
//---------------------------------------------------------------------------

bool CSVFileParser :: ParseNext( vector <string> & data ) {
	return mParser->ParseNext( data );
}

//---------------------------------------------------------------------------
// Forward to stream parser
//---------------------------------------------------------------------------

unsigned int CSVFileParser :: LineNo() const {
	return mParser->LineNo();
}

string CSVFileParser :: RawLine() const {
	return mParser->RawLine();
}

//---------------------------------------------------------------------------

}	// namespace

//----------------------------------------------------------------------------
// tests
//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_csv" );

DEFTEST( LineTest ) {
	CSVLineParser lp;
	string s = "foo,bar";
	vector <string> v;
	lp.Parse( s, v );
	FAILNE( v.size(), 2 );
	FAILNE( v.at(0), "foo" );
	FAILNE( v.at(1), "bar" );
}

DEFTEST( SepTest ) {
	CSVLineParser lp('|');
	string s = "foo|bar";
	vector <string> v;
	lp.Parse( s, v );
	FAILNE( v.size(), 2 );
	FAILNE( v.at(0), "foo" );
	FAILNE( v.at(1), "bar" );
}

DEFTEST( StreamTest ) {
	std::istringstream is( "foo,bar\n1,2,3\n" );
	CSVStreamParser sp( is );
	vector <string> v;
	bool ok = sp.ParseNext( v );
	FAILNE( ok, true );
	FAILNE( v.size(), 2 );
	FAILNE( v.at(0), "foo" );
	FAILNE( v.at(1), "bar" );
	ok = sp.ParseNext( v );
	FAILNE( ok, true );
	FAILNE( v.size(), 3 );
	ok = sp.ParseNext( v );
	FAILNE( ok, false );
}

// there is a bug with field counts being different with quoted
// and non-quoted input when there is a trailing comma
// now (hopefuly) fixed

DEFTEST( FieldCount ) {
	CSVLineParser lp;
	vector <string> v;
	string s = "1,2,";
	lp.Parse( s, v );
	FAILNE( v.size(), 3 );
	FAILNE( v.at(0), "1" );
	FAILNE( v.at(1), "2" );
	FAILNE( v.at(2), "" );
	s = "\"1\",\"2\",";
	lp.Parse( s, v );
	FAILNE( v.size(), 3 );
	FAILNE( v.at(0), "1" );
	FAILNE( v.at(1), "2" );
	FAILNE( v.at(2), "" );

}

#endif

// end
