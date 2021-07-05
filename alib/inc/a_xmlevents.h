//---------------------------------------------------------------------------
// a_xmlparser.h
//
// simple event driven parser
//
// This software is licensed under the Apache 2.0 license. You should find
// a copy of the license in the file LICENSE.TXT. If this file is missing,
// a copy can be obtained from www.apache.org.
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_XMLEVENTS_H
#define INC_A_XMLEVENTS_H

#include "a_base.h"

//---------------------------------------------------------------------------

namespace ALib {

//----------------------------------------------------------------------------
// Simple event driven parser. Note the tree parser does not use this yet.
//----------------------------------------------------------------------------

class XMLEventParser {

	CANNOT_COPY( XMLEventParser );

	public:

		// name/value pairs
		struct Attribute {
			std::string mName, mValue;
			Attribute( const std::string & name, const std::string & val )
				: mName( name ), mValue( val ) {
			}
		};

		typedef std::vector<Attribute> Attributes;

		XMLEventParser();
		virtual ~XMLEventParser();

		bool ParseString( const std::string & xml );
		bool ParseStream( std::istream & xml );

		std::string ErrorMsg() const;
		int ErrorCode() const;
		int ErrorLine() const;

		virtual void OnElementStart( const std::string & name,
								const Attributes  & attrs ) = 0;
		virtual void OnElementEnd( const std::string & name ) = 0;
		virtual void OnText( const std::string & name ) = 0;

	private:

		class ParserImpl * mImpl;

};

//---------------------------------------------------------------------------

} // end of namespace

#endif

