//---------------------------------------------------------------------------
// a_xmlparser.cpp
//
// simple event-driven XML parser wrapper for ALib.
// This implementtaion used the Expat parser.
//
// This software is licensed under the Apache 2.0 license. You should find
// a copy of the license in the file LICENSE.TXT. If this file is missing,
// a copy can be obtained from www.apache.org.

// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include <fstream>
#include <stack>
#include "a_collect.h"
#include "a_xmlevents.h"
#include "a_except.h"
#include "a_str.h"
#include "expat.h"
using std::string;
using std::vector;

namespace ALib {

//---------------------------------------------------------------------------
// The following is a wrapper for the Expat parser
//----------------------------------------------------------------------------

class ParserImpl {

	CANNOT_COPY( ParserImpl );

	public:

		ParserImpl( XMLEventParser * p );
		~ParserImpl();

		bool Parse( const std::string & s );

		string ErrorMsg() const {
			return mErrorMsg;
		}

		int ErrorCode() const {
			return mErrorCode;
		}

		int ErrorLine() const {
			return mErrorLine;
		}

	private:

		static void StartElementHandler( void * parser,
											const XML_Char * name,
											const XML_Char ** atts );
		static void EndElementHandler( void * parser,
									const XML_Char *name);
		static void CharDataHandler( void * parser,
										const XML_Char *s,
										int len);

		static XMLEventParser * EVP( void * p );

		XML_Parser mExpat;
		XMLEventParser * mEventParser;
		string mErrorMsg;
		int mErrorLine, mErrorCode;

};

//----------------------------------------------------------------------------

/// need to kow which event parser is wrapping us
ParserImpl :: ParserImpl( XMLEventParser * ep )
	: mExpat( 0 ), mEventParser( ep )  {
}

/// free up any expat object
ParserImpl :: ~ParserImpl() {
	if ( mExpat ) {
		XML_ParserFree( mExpat );
	}
}

/// convenience function to get event parser pointer
XMLEventParser * ParserImpl :: EVP( void * p ) {
	return ((ParserImpl *) p)->mEventParser;
}

/// create attribute list then call event parser's event
void ParserImpl :: StartElementHandler( void * parser,
											const XML_Char * name,
											const XML_Char ** atts ) {
	XMLEventParser::Attributes a;

	unsigned int i = 0;
	while( atts[i] ) {
		a.push_back( XMLEventParser::Attribute( atts[i], atts[i+1] ) );
		i += 2;
	}

	EVP(parser)->OnElementStart( name, a );
}

/// fire event parsers event
void ParserImpl :: EndElementHandler( void * parser,
										const XML_Char *name) {
	EVP(parser)->OnElementEnd( name );
}

/// fire event parsers event
void ParserImpl :: CharDataHandler( void * parser,
										const XML_Char *s,
										int len ) {
	EVP(parser)->OnText( string( s, len ) );
}

/// do actual parse, firing events a& returning true if all went well
/// need to create expat parser each time we dparse as its a one shot device
bool ParserImpl :: Parse( const string & s ) {

	if ( mExpat ) {
		XML_ParserFree( mExpat );
	}

	mExpat = XML_ParserCreate( 0 );
	XML_SetElementHandler( mExpat, StartElementHandler, EndElementHandler );
	XML_SetCharacterDataHandler( mExpat, CharDataHandler );
	XML_SetUserData( mExpat, this );

	unsigned int err = XML_Parse(  mExpat, s.c_str(), s.size(), 1 );
	if ( err == 0 ) {
		mErrorCode = XML_GetErrorCode( mExpat );
		mErrorMsg  = XML_ErrorString( XML_Error( err ) );
		mErrorLine = XML_GetCurrentLineNumber( mExpat );
	}

	return err != 0;
}

//----------------------------------------------------------------------------
// The event parser implements a few methods but to get it to do anything
// the virtual On... methods must be overridden
//----------------------------------------------------------------------------

XMLEventParser :: XMLEventParser() {
	mImpl = new ParserImpl( this );
}

XMLEventParser :: ~XMLEventParser() {
	delete mImpl;
}

bool XMLEventParser :: ParseString( const string & xml ) {
	return mImpl->Parse( xml );
}

// TODO (neilb#1#): need better way of reading stream
bool XMLEventParser :: ParseStream( std::istream & xml ) {
	string text, line;
	while( std::getline( xml, line ) ) {
		text += line;
		text += '\n';
	}
	return ParseString( text );
}

string XMLEventParser :: ErrorMsg() const {
	return mImpl->ErrorMsg();
}

int XMLEventParser :: ErrorCode() const {
	return mImpl->ErrorCode();
}

int XMLEventParser :: ErrorLine() const {
	return mImpl->ErrorLine();
}

} // end of namespace

//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_xmlevents" );

// a very simple parser
struct MyXMLParser : public XMLEventParser {

	void OnElementStart( const std::string & name,
							const Attributes  & attrs ) {
		mStr += name;
	}

	void OnElementEnd( const std::string & name ) {
		mStr += "/" + name;
	}

	void OnText( const std::string & text ) {
		mStr += Trim( text );
	}

	string mStr;
};


// test the simple parser
DEFTEST( EventParser ) {
	MyXMLParser mp;
	string xml= "<foo></foo>";
	mp.ParseString( xml );
	FAILNE( mp.mStr, "foo/foo" );
	xml ="<foo>hello</foo>";
	mp.mStr = "";
	mp.ParseString( xml );
	FAILNE( mp.mStr, "foohello/foo" );
}

#endif

//----------------------------------------------------------------------------

// end
