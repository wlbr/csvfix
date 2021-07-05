//---------------------------------------------------------------------------
// a_xmlparser.cpp
//
// XML parser wrapper for ALib.
// This implementtaion used the Expat parser.
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include <fstream>
#include <stack>
#include "a_collect.h"
#include "a_xmlparser.h"
#include "a_except.h"
#include "a_str.h"
#include "expat.h"
using std::string;
using std::vector;

using namespace std;

//---------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// Implementtaion using the Expat parser
//---------------------------------------------------------------------------

class  XMLTreeParser::Impl {

	public:

		Impl();
		~Impl();

		XMLElement * Parse( const std::string & s );
		XMLElement * ParseFile( const std::string & file );
		XMLElement * ParseStream( std::istream & is );

		const string & File() const {
			return mFile;
		}

		const string & ErrMsg() const {
			return mErrMsg;
		}

		const string & ErrContext() const {
			return mErrContext;
		}

		unsigned int ErrLine() const {
			return mErrLine;
		}

		void IgnoreWhitespace( bool ignore ) {
			mIgnoreWhitespace = ignore;
		}

	private:

		void Init();

		static void StartElementHandler( void * userData,
											const XML_Char * name,
											const XML_Char ** atts );

		static void EndElementHandler( void * userData,
									const XML_Char *name);

		static void CharDataHandler( void *userData,
										const XML_Char *s,
										int len);

		XMLElement * mRoot;
		XML_Parser mExpat;					// pointer to Expat parser
		std::stack <XMLElement *> mStack;
		string mErrMsg, mErrContext, mFile;
		unsigned int mErrLine;
		bool mIgnoreWhitespace;

};

//---------------------------------------------------------------------------
// Don't create expat parser here because each such parser is a one-shot
// affair that can only be used for a single parse.
//---------------------------------------------------------------------------

 XMLTreeParser::Impl :: Impl()
	: mRoot(0), mExpat(0), mErrLine(0), mIgnoreWhitespace( true ) {
}

//---------------------------------------------------------------------------
// Delete anything we have hung on to.
//---------------------------------------------------------------------------

 XMLTreeParser::Impl :: ~Impl() {
	if ( mRoot ) {
		delete mRoot;
	}
	if ( mExpat ) {
		XML_ParserFree( mExpat );
	}
}

//---------------------------------------------------------------------------
// Create the expat parser, junking any existing one, and set the handlers
//---------------------------------------------------------------------------

void  XMLTreeParser::Impl :: Init() {
	if ( mExpat ) {
		XML_ParserFree( mExpat );
	}
	mExpat = XML_ParserCreate( 0 );
	Clear( mStack );
	mErrMsg = "";
	mErrLine = 0;
	XML_SetElementHandler( mExpat, StartElementHandler, EndElementHandler );
	XML_SetCharacterDataHandler( mExpat, CharDataHandler );
	XML_SetUserData( mExpat, this );
}

//---------------------------------------------------------------------------
// Parse text, returning the root of the XML tree on success, relinquishing
// ownership of the root & all its nodes. If parse fails, junk any partly
// built tree, save context info and return null.
//---------------------------------------------------------------------------

const int MAX_CONTEXT = 32;		// max length of XML error context

XMLElement *  XMLTreeParser::Impl :: Parse( const string & s ) {
	Init();
	unsigned int err = XML_Parse(  mExpat, s.c_str(), s.size(), 1 );
	if ( err == 0 ) {
		// there was an error - recorde details
		err = XML_GetErrorCode( mExpat );
		mErrMsg = XML_ErrorString( XML_Error( err ) );
		mErrLine = XML_GetCurrentLineNumber( mExpat );
		int offset, size;
		const char * buf = XML_GetInputContext( mExpat, &offset, &size );
		mErrContext = string( buf + offset, Min( size - offset, MAX_CONTEXT ) );
		delete mRoot;
		mRoot = 0;
	}
	XMLElement * e = mRoot;		// parse will have set this
	mRoot = 0;
	return e;
}

//---------------------------------------------------------------------------
// Parse input stream. Currently reads whole streem contents into memory
// and then calls Parse() - not very efficient!
//---------------------------------------------------------------------------

XMLElement * XMLTreeParser::Impl :: ParseStream( istream & is ) {
	string s, line;
	while( std::getline( is, line ) ) {
		s += line + "\n";
	}
	return Parse( s );
}

//---------------------------------------------------------------------------
// Parse file by using stream parse
//---------------------------------------------------------------------------

XMLElement *  XMLTreeParser::Impl :: ParseFile( const string & file ) {
	mFile = file;
	std::ifstream f( file.c_str() );
	if ( ! f.is_open() ) {
		mErrMsg = "Cannot open file " + SQuote( file );
		return 0;
	}
	return ParseStream( f );
}

//---------------------------------------------------------------------------
// Start tag handler creates element and adds it as child to element on
// top of stack, then pushes new element as new stack top
//---------------------------------------------------------------------------

void  XMLTreeParser::Impl :: StartElementHandler( void * userData,
											const XML_Char * name,
											const XML_Char ** atts ) {

	Impl * parser = static_cast< XMLTreeParser::Impl*>( userData );
	int line = XML_GetCurrentLineNumber( parser->mExpat );

	XMLElement * e = new XMLElement( name, line, parser->mFile );

	if ( parser->mRoot == 0 ) {
		parser->mRoot = e;
	}

	unsigned int i = 0;
	while( atts[i] ) {
		e->AddAttr( atts[i], atts[i+1] );
		i += 2;
	}

	if ( parser->mStack.size() ) {
		parser->mStack.top()->AddChild( e );
	}
	parser->mStack.push( e );
}

//---------------------------------------------------------------------------
// End tag handler pops element stack
//---------------------------------------------------------------------------

void  XMLTreeParser::Impl :: EndElementHandler( void * userData,
									const XML_Char *name) {
	Impl * parser = static_cast< XMLTreeParser::Impl*>( userData );
	if ( parser->mStack.size() == 0
		|| parser->mStack.top()->Name() != name ) {
		ATHROW( "Invalid end tag " << SQuote( name ) );
	}
	parser->mStack.pop();
}

//---------------------------------------------------------------------------
// Text handler strips any leading whitespace and then adds text as child
// to top of stack.
// TODO (neilb#3#): Maybe have switch to allow retention of leading spaces
//---------------------------------------------------------------------------

void  XMLTreeParser::Impl :: CharDataHandler( void *userData,
											const XML_Char *s,
											int len) {
	Impl * parser = static_cast< XMLTreeParser::Impl*>( userData );
	int line = XML_GetCurrentLineNumber( parser->mExpat );

//	string dbg( s, len );
//	std::cerr << "----[" << dbg << "]\n";

	// remove leading spaces
	const char * p = s;
	while( parser->mIgnoreWhitespace && len && *p <= ' ' ) {
		p++;
		len--;
	}

	if ( len == 0 ) {
		return;		// don't add wgitespace strings
	}

	string t( p, len );
	if ( parser->mStack.size() == 0 ) {
		ATHROW( "Unexpected text " << SQuote(t) );
	}
	parser->mStack.top()->AddChild( new XMLText( t, line, parser->mFile ) );
}

//---------------------------------------------------------------------------
// Actual parser - jus pass params to implementation.
//---------------------------------------------------------------------------

 XMLTreeParser ::  XMLTreeParser() {
	mImpl = new  XMLTreeParser::Impl;
}

//---------------------------------------------------------------------------
// Junk implementation
//---------------------------------------------------------------------------

 XMLTreeParser :: ~ XMLTreeParser() {
	delete mImpl;
}

//---------------------------------------------------------------------------
// Parse just passed through to implememntation
//---------------------------------------------------------------------------

XMLElement *  XMLTreeParser :: Parse( const string & xml ) {
	return mImpl->Parse( xml );
}

//---------------------------------------------------------------------------
// Parse named file
//---------------------------------------------------------------------------

XMLElement *  XMLTreeParser :: ParseFile( const string & xmlfile ) {
	return mImpl->ParseFile( xmlfile );
}

//---------------------------------------------------------------------------
// Parse input stream
//---------------------------------------------------------------------------

XMLElement *  XMLTreeParser :: ParseStream( std::istream & is) {
	return mImpl->ParseStream( is );
}

//---------------------------------------------------------------------------
// Get text from input buffer where error occurred
//---------------------------------------------------------------------------

string  XMLTreeParser :: ErrorContext() const {
	return mImpl->ErrContext();
}

//---------------------------------------------------------------------------
// Descriptive (?) error message
//---------------------------------------------------------------------------

string  XMLTreeParser :: ErrorMsg() const {
	return mImpl->ErrMsg();
}

//---------------------------------------------------------------------------
// Line error is on
//---------------------------------------------------------------------------

unsigned int  XMLTreeParser :: ErrorLine() const {
	return mImpl->ErrLine();
}

//---------------------------------------------------------------------------
// File being parsed - may be empty
//---------------------------------------------------------------------------

string  XMLTreeParser :: File() const {
	return mImpl->File();
}

//----------------------------------------------------------------------------
// Ignore text events that contain only whitespace characters
//----------------------------------------------------------------------------

void XMLTreeParser :: IgnoreWhitespace( bool ignore ) {
	mImpl->IgnoreWhitespace( ignore );
}

//----------------------------------------------------------------------------


} // end of namespace


//----------------------------------------------------------------------------
// Testing
//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_xmlparse" );

DEFTEST( Simple ) {
	string xml = "<root><name>fred</name></root>";
	XMLTreeParser tp;
	XMLElement * e = tp.Parse( xml );
	FAILIF( e == 0 );
	FAILNE( e->ChildCount(), 1 );
	const XMLElement * ce = e->ChildElement( 0 );
	FAILIF( ce == 0 );
	FAILNE( ce->Name(), "name" );
	FAILNE( ce->ChildCount(), 1 );
	const XMLText * t = ce->ChildText( 0 );
	FAILIF( t == 0 );
	string s = t->Text();
	FAILNE( s, "fred" );
}

DEFTEST( MultiLine ) {
	string xml = "<root><name>fred\nbloggs</name></root>";
	XMLTreeParser tp;
	tp.IgnoreWhitespace( false );
	XMLElement * e = tp.Parse( xml );
	FAILIF( e == 0 );
	FAILNE( e->ChildCount(), 1 );
	const XMLElement * ce = e->ChildElement( 0 );
	FAILIF( ce == 0 );
	FAILNE( ce->Name(), "name" );
	FAILNE( ce->ChildCount(), 3 );
	const XMLText * t = ce->ChildText( 0 );
	FAILIF( t == 0 );
	string s = t->Text();
	FAILNE( s, "fred" );
	t = ce->ChildText( 1 );
	FAILIF( t == 0 );
	s = t->Text();
	FAILNE( s, "\n" );
}

DEFTEST( Blank ) {
	string xml = "<root><name> </name></root>";
	XMLTreeParser tp;
	tp.IgnoreWhitespace( false );
	XMLElement * e = tp.Parse( xml );
	FAILIF( e == 0 );
	FAILNE( e->ChildCount(), 1 );
	const XMLElement * ce = e->ChildElement( 0 );
	FAILIF( ce == 0 );
	FAILNE( ce->Name(), "name" );
	FAILNE( ce->ChildCount(), 1 );
	const XMLText * t = ce->ChildText( 0 );
	FAILIF( t == 0 );
	string s = t->Text();
	FAILNE( s, " " );
}

#endif

// end
