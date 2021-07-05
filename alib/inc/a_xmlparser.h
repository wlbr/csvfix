//---------------------------------------------------------------------------
// a_xmlparser.h
//
// XML parser wrapper for alib. No dependency on actuial parser used here.
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_XMLPARSER_H
#define INC_A_XMLPARSER_H

#include "a_base.h"
#include "a_xmltree.h"

//---------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// Parse XML text producing a tree who's root is returned by the parse
// functions.  We hide the parser umplementatiom using a PIMPL so that we
// can re-implement with a different XML parser library if needs be.
//---------------------------------------------------------------------------

class XMLTreeParser {

	class Impl;

	public:

		XMLTreeParser();
		virtual ~XMLTreeParser();

		XMLElement * Parse( const std::string & xmltext );
		XMLElement * ParseFile( const std::string & xmlfile );
		XMLElement * ParseStream( std::istream & is );

		std::string ErrorMsg() const;
		unsigned int ErrorLine() const;
		std::string ErrorContext() const;
		std::string File() const;

		void IgnoreWhitespace( bool ignore );

	private:

		Impl * mImpl;
};


//---------------------------------------------------------------------------

} // end of namespace

#endif

