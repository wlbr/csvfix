//---------------------------------------------------------------------------
// a_xmltree.h
//
// xml tree for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_XMLTREE_H
#define INC_A_XMLTREE_H

#include "a_base.h"
#include "a_shstr.h"

//---------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------


class XMLNode {

	friend class XMLElement;

	public:

		XMLNode( unsigned int line, const std::string & file );
		virtual ~XMLNode() = 0;

		std::string File() const;
		unsigned int Line() const;

		const class XMLElement * Parent() const;

	private:

		void SetParent( const XMLElement * e );

		unsigned int mLine;
		SharedString mFile;
		const XMLElement * mParent;


};

//---------------------------------------------------------------------------

class XMLText : public XMLNode {

	public:

		XMLText( const std::string & txt,
					unsigned int line = 0, const std::string & file = "" );

		virtual ~XMLText();

		const std::string & Text() const;

	public:

		std::string mText;
};

//---------------------------------------------------------------------------

class XMLElement : public XMLNode {

	public:

		XMLElement( const std::string & name,
						unsigned int line = 0, const std::string & file = "" );
		~XMLElement();

		const std::string & Name() const;

		void AddAttr( const std::string & aname, const std::string & aval );

		unsigned int AttrCount() const;
		const std::string & AttrName( unsigned int i ) const;
		const std::string & AttrValue( const std::string & aname ) const;
		const std::string & AttrValue( const std::string & aname,
										const std::string & def ) const;

		bool HasAttr( const std::string & aname ) const;
		bool HasText() const;

		void AddChild( XMLNode * n );

		void Replace( XMLElement * e );

		unsigned int ChildCount() const;
		const XMLNode * ChildAt( unsigned int i ) const;

		const XMLElement * ChildElement( const std::string & name ) const;
		const XMLElement * ChildElement( unsigned int i) const;
		const XMLText * ChildText( unsigned int i ) const;

		void CheckAttrs( const std::string & req,
							const std::string & opt ) const;

		void Error( const std::string & emsg ) const;

		void DumpOn( std::ostream & os, bool recurse = false ) const;

	private:

		std::string Location() const;

		std::string mName;
		std::vector <XMLNode *> mKids;

		struct AttrVal {
			std::string mName, mVal;
			AttrVal( const std::string & a, const std::string & v )
				: mName( a ), mVal( v ) {}
		};

		std::vector <AttrVal> mAttrs;
};


//---------------------------------------------------------------------------

}		// end namespace

#endif

