//---------------------------------------------------------------------------
// csved_toxml.h
//
// convert CSV to XML
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_TOXML_H
#define INC_CSVED_TOXML_H

#include "a_base.h"
#include "a_slice.h"
#include "csved_command.h"
#include "csved_ioman.h"

namespace CSVED {

//---------------------------------------------------------------------------
// Base class for XML specification tree
//----------------------------------------------------------------------------

class XMLSpecNode {

	public:

		XMLSpecNode( int ind );
		virtual ~XMLSpecNode() = 0;
		virtual XMLSpecNode * Parent();
		void SetParent( XMLSpecNode * p );
		int Indent() const;
		virtual void Dump() = 0;

	private:

		int mIndent;
		XMLSpecNode * mParent;
};

//----------------------------------------------------------------------------
// Tree node that specifies a tag. Tags have a name, optional attributes
// and optional child tagsd. They also have a group which identifies which
// CSV rows are to be used to populate the children.
//----------------------------------------------------------------------------

class XMLSpecTag : public XMLSpecNode {

	public:

		XMLSpecTag( int ind, const std::string & name );
		~XMLSpecTag();

		void AddChild( XMLSpecNode * p );
		XMLSpecNode * ChildAt( unsigned int i ) const;
		unsigned int ChildCount() const;

		struct Attrib {
			Attrib( const std::string & name, unsigned int idx )
				: mName( name ), mIndex( idx ) {}
			std::string mName;
			unsigned int mIndex;
		};

		const std::string & Name() const {
			return mTag;
		}

		void AddAttrib( const Attrib & a );
		const Attrib & AttribAt( unsigned int i ) const;
		unsigned int AttribCount() const;

		void SetGroup( const std::string & grp );
		unsigned int GroupCount() const;
		unsigned int GroupAt( unsigned int i ) const;

		void Dump();

	private:

		std::string mTag;
		std::vector <XMLSpecNode *> mKids;
		std::vector <Attrib> mAttribs;
		std::vector <int> mGroup;

};

//----------------------------------------------------------------------------
// Text node specifies XML text entity, which may need CDATA wrapping
//----------------------------------------------------------------------------

class XMLSpecText : public XMLSpecNode {

	public:

		XMLSpecText( int ind, unsigned int idx, bool cdata );

		unsigned int Index() const;

		bool IsCDATA() const {
			return mIsCDATA;
		}

		void Dump();

	private:

		unsigned int mIndex;
		bool mIsCDATA;

};


//----------------------------------------------------------------------------
// Command to convert CSV to XML
//----------------------------------------------------------------------------

class ToXMLCommand : public Command {

	public:

		ToXMLCommand( const std::string & name,
						const std::string & desc );
		~ToXMLCommand();

		int Execute( ALib::CommandLine & cmd );

	private:

		typedef ALib::Slice <CSVRow> InSlice;
		typedef std::vector <InSlice> SliceVec;

		void ProcessFlags( const ALib::CommandLine & cl );
		void MakeTable( IOManager & io );
		void OuputTableRow( std::ostream & os, const CSVRow & row );

		XMLSpecNode * MakeTagSpec( int indent,
									const std::vector <std::string> & toks);
		XMLSpecNode * MakeTextSpec( int indent,
									const std::vector <std::string> & toks,
									bool cdata );
		XMLSpecTag *  ReadSpec( const std::string & filename );

		void MakeXML( IOManager & io, const XMLSpecTag * t,
						const InSlice & is, unsigned int indent );
		void MakeMySlices( const XMLSpecTag * t,
								const InSlice & is, SliceVec & mys );
		void MakeAttribs( IOManager & io, const XMLSpecTag * t,
								const InSlice & is );
		void MakeText( IOManager & io, const XMLSpecText * t,
						const InSlice & is, unsigned int indent );

		XMLSpecNode * MakeSpecNode( int indent,
									const std::vector <std::string> &tokens );

		std::string Indent( unsigned int i ) const;

		std::string mXMLSpec;

		unsigned int mIndent;
		bool mUseTabs, mEndTag;

};


//------------------------------------------------------------------------

}	// end namespace

#endif

