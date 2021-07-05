//---------------------------------------------------------------------------
// a_xmltree.cpp
//
// xml tree for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_xmltree.h"
#include "a_collect.h"
#include "a_except.h"
#include "a_str.h"
#include <assert.h>

using std::string;
using std::vector;

//---------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// XML tree node abstract base class. Records line and name of source file.
// Filename is stored as SharedString to save some space.
//---------------------------------------------------------------------------

XMLNode :: XMLNode( unsigned int line, const string & file  )
	: mLine( line ), mFile( file ), mParent( 0 ) {
}

XMLNode :: ~XMLNode() {
	// nothing
}

string XMLNode :: File() const {
	return mFile.Str();
}

unsigned int XMLNode :: Line() const {
	return mLine;
}


//---------------------------------------------------------------------------
// Parent must be elemnet, not node
//---------------------------------------------------------------------------

const XMLElement * XMLNode :: Parent() const {
	return mParent;
}

//---------------------------------------------------------------------------
// Element needs to be able to set parent after node is constructed
//---------------------------------------------------------------------------

void XMLNode :: SetParent( const XMLElement * e ) {
	assert( mParent == 0 );	// no parent already
	mParent = e;
}

//---------------------------------------------------------------------------
// Elements must have name
//---------------------------------------------------------------------------

XMLElement :: XMLElement( const string & name,
							unsigned int line, const string & file )
	: XMLNode( line, file ), mName( name ) {
}

//---------------------------------------------------------------------------
// Free up all child elements
//---------------------------------------------------------------------------

XMLElement :: ~XMLElement() {
	FreePtrs( mKids );
}


//---------------------------------------------------------------------------
// If there is a file/line, return string giviong location.
//---------------------------------------------------------------------------

string XMLElement :: Location() const {
	if ( File() == "" ) {
		return "";
	}
	else {
		return " in " + File() + " at line " + Str( Line() );
	}
}

//---------------------------------------------------------------------------
// XML element name
//---------------------------------------------------------------------------

const string & XMLElement :: Name() const {
	return mName;
}

//---------------------------------------------------------------------------
// Add attribute, which must not pre-exist
//---------------------------------------------------------------------------

void XMLElement :: AddAttr( const string & aname, const string & aval ) {
	if ( HasAttr( aname ) ) {
		ATHROW( "Duplicate attribute name " << SQuote( aname ) );
	}
	mAttrs.push_back( AttrVal( aname, aval ) );
}

//---------------------------------------------------------------------------
// How many attributes?  Might be zero.
//---------------------------------------------------------------------------

unsigned int XMLElement :: AttrCount() const {
	return mAttrs.size();
}

//---------------------------------------------------------------------------
// Does element have named attribute?
//---------------------------------------------------------------------------

bool XMLElement :: HasAttr( const string & aname ) const {
	for ( unsigned int i = 0; i < mAttrs.size(); i++ ) {
		if ( mAttrs[i].mName == aname ) {
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------------
// Get name of attribute at index.
//---------------------------------------------------------------------------

const string & XMLElement :: AttrName( unsigned int i ) const {
	if ( i >= AttrCount() ) {
		ATHROW( "Attribute index " << SQuote( Str(i) ) << " out of range" );
	}
	return mAttrs[i].mName;
}

//---------------------------------------------------------------------------
// Value of named attribute. May be empty.
//---------------------------------------------------------------------------

const string & XMLElement :: AttrValue( const string & aname ) const {
	for ( unsigned int i = 0; i < mAttrs.size(); i++ ) {
		if ( mAttrs[i].mName == aname ) {
			return mAttrs[i].mVal;
		}
	}
	ATHROW( "Unknown attribute name " << SQuote( aname ) << Location() );
}

//---------------------------------------------------------------------------
// Get attribute value or default if no attribute exists
//---------------------------------------------------------------------------

const string & XMLElement :: AttrValue( const string & aname,
										const string & def ) const {
	for ( unsigned int i = 0; i < mAttrs.size(); i++ ) {
		if ( mAttrs[i].mName == aname ) {
			return mAttrs[i].mVal;
		}
	}
	return def;
}

//---------------------------------------------------------------------------
// Add child element
//---------------------------------------------------------------------------

void XMLElement :: AddChild( XMLNode * n ) {
	n->SetParent( this );
	mKids.push_back( n );
}

//---------------------------------------------------------------------------
// How many child nodes?
//---------------------------------------------------------------------------

unsigned int XMLElement :: ChildCount() const {
	return mKids.size();
}

//---------------------------------------------------------------------------
// Get child node at index
//---------------------------------------------------------------------------

const XMLNode * XMLElement :: ChildAt( unsigned int i ) const {
	if ( i >= ChildCount() ) {
		ATHROW( "Node index " << SQuote(Str(i)) << " out of range" );
	}
	return mKids[i];
}

//---------------------------------------------------------------------------
// Return pointer to first child element with name, or null pointer if
// such a child does not exist.
//---------------------------------------------------------------------------

const XMLElement * XMLElement :: ChildElement( const string & name ) const {
	for ( unsigned int i = 0; i < mKids.size(); i++ ) {
		const XMLElement * e = dynamic_cast <XMLElement *>( mKids[i] );
		if ( e != 0 && e->Name() == name ) {
			return e;
		}
	}
	return 0;
}

//---------------------------------------------------------------------------
// If indexed child is element, return pointer to it else return null
//---------------------------------------------------------------------------

const XMLElement * XMLElement :: ChildElement( unsigned int i) const {
	const XMLElement * e = dynamic_cast <const XMLElement *>( ChildAt(i) );
	return e;
}

//---------------------------------------------------------------------------
// As above, but for text node
//---------------------------------------------------------------------------

const XMLText * XMLElement :: ChildText( unsigned int i ) const {
	const XMLText * t = dynamic_cast <const XMLText *>( ChildAt(i) );
	return t;
}

//---------------------------------------------------------------------------
// Check element has required attributes and only contains attributes
// inn the required and optional lists.
//---------------------------------------------------------------------------

void XMLElement :: CheckAttrs( const string & req,
								const string & opt ) const {
	CommaList cl;
	if ( req != "" ) {
		cl.Append( req );
		for ( unsigned int i = 0; i < cl.Size(); i++ ) {
			if ( ! HasAttr( cl.At( i ) ) ) {
				Error( "Missing required attribute " + SQuote( cl.At( i ) ) );
			}
		}
	}

	if ( opt != "" ) {
		cl.Append( opt );
	}

	for ( unsigned int i = 0; i < mAttrs.size(); i++ ) {
		if ( ! cl.Contains( mAttrs[i].mName ) ) {
			Error( "Invalid attribute " + SQuote( mAttrs[i].mName  ));
		}
	}
}

//----------------------------------------------------------------------------
// Replace this XML node with another. The  replacement must be the root
// of an XML tree. Ownership is taken by the tree the replacement takes
// place in.
//----------------------------------------------------------------------------

void XMLElement :: Replace( XMLElement * e ) {
	XMLElement * p = const_cast<XMLElement *>( Parent() );
	if ( p == 0 ) {
		Error( "Cannot replace XML root" );
	}
	for ( unsigned int i = 0; i <p->ChildCount(); i++ ) {
		XMLElement * rp = dynamic_cast <XMLElement *>(p->mKids[i] );
		if ( rp != 0 && rp == this ) {
			p->mKids[i] = e;
			delete rp;
			return;
		}
	}
	Error( "Replace XML failed" );
}

//---------------------------------------------------------------------------
// Report error via element to get location info
//---------------------------------------------------------------------------

void XMLElement :: Error( const std::string & emsg ) const {
	ATHROW( emsg << Location() );
}

//---------------------------------------------------------------------------
// Return true if element has at least one text node
//---------------------------------------------------------------------------

bool XMLElement :: HasText() const {
	for ( unsigned int i = 0; i < mKids.size(); i++ ) {
		if ( dynamic_cast <XMLText *> ( mKids[i] ) ) {
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------------
// Debug dump - this may not produce "real" XML
//---------------------------------------------------------------------------

void XMLElement :: DumpOn( std::ostream & os, bool recurse  ) const {
	os << "<" << Name();
	for ( unsigned int i = 0; i < AttrCount(); i++ ) {
		os << " " << AttrName(i) << "=\"";
		os << AttrValue( AttrName(i) ) << "\"";
	}
	if ( ! recurse ) {
		os << "/>\n";
	}
	else {
		os << ">\n";
		for ( unsigned int i = 0; i < ChildCount(); i++ ) {
			const XMLElement * ce = ChildElement( i ) ;
			if ( ce ) {
				ce->DumpOn( os, recurse );
			}
			const XMLText * ct = ChildText( i );
			if ( ct ) {
				os << LTrim( ct->Text() ) << "\n";
			}
		}
		os << "</" << Name() << ">\n";
	}
}

//---------------------------------------------------------------------------
// XML text node. This class does not handle trimming of bogus spaces and
// newlines passed by an XML parser.
//---------------------------------------------------------------------------

XMLText :: XMLText( const string & txt, unsigned int line,
								const string & file  )
	: XMLNode( line, file ), mText( txt ) {
}

XMLText :: ~XMLText() {
	// nothing
}

const string & XMLText :: Text() const {
	return mText;
}

} // end namespace

// end
