//---------------------------------------------------------------------------
// csved_toxml.cpp
//
// convert  CSV to XML
// ??? rather complicated & needs refactoring ???
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "csved_toxml.h"
#include "csved_cli.h"
#include "csved_strings.h"
#include "a_collect.h"
#include "a_html.h"
#include <fstream>

using std::string;
using std::vector;

namespace CSVED {


//----------------------------------------------------------------------------
// Add the to_xml command
//----------------------------------------------------------------------------

static RegisterCommand <ToXMLCommand> rc1_(
	CMD_TOXML,
	"convert CSV to XML"
);

//----------------------------------------------------------------------------
// Strings for config file elements
//----------------------------------------------------------------------------

const char * const TAG_STR 		= "tag";
const char * const GROUP_STR 		= "group";
const char * const ATTRIB_STR 		= "attrib";
const char * const TEXT_STR 		= "text";
const char * const CDATA_STR 		= "cdata";

const char COMMENT_CHAR 			= '#';

//----------------------------------------------------------------------------
// Create spec node with no parent. The parent will be set later when the
// node is added to the soec tree.
//----------------------------------------------------------------------------

XMLSpecNode :: XMLSpecNode( int ind )
	: mIndent(ind ), mParent( 0 ) {
}

//----------------------------------------------------------------------------
// Get parent, or 0 if none, which is case for tree root node
//----------------------------------------------------------------------------

XMLSpecNode * XMLSpecNode :: Parent() {
	return mParent;
}

//----------------------------------------------------------------------------
// Set parent of this node
//----------------------------------------------------------------------------

void XMLSpecNode :: SetParent( XMLSpecNode * p ) {
	mParent = p;
}

//----------------------------------------------------------------------------
// Get level of indentation of this node in the textual input. This is then
// used to determine parantage of node.
//----------------------------------------------------------------------------

int XMLSpecNode :: Indent() const {
	return mIndent;
}

//----------------------------------------------------------------------------
// Do-nothing pure virtual dtor
//----------------------------------------------------------------------------

XMLSpecNode :: ~XMLSpecNode() {
}

//----------------------------------------------------------------------------
// Tag spec node describeas a tag with aname, attributes kids etc.
//----------------------------------------------------------------------------

XMLSpecTag :: XMLSpecTag( int ind, const string & name )
	: XMLSpecNode( ind ), mTag( name ) {
}

//----------------------------------------------------------------------------
// Displose of any child nodes
//----------------------------------------------------------------------------

XMLSpecTag ::~XMLSpecTag() {
	ALib::FreeClear( mKids );
}

//----------------------------------------------------------------------------
// add a child, setting its parent
//----------------------------------------------------------------------------

void XMLSpecTag :: AddChild( XMLSpecNode * p ) {
	mKids.push_back( p );
	p->SetParent( this );
}

//----------------------------------------------------------------------------
// Get child by index
//----------------------------------------------------------------------------

XMLSpecNode * XMLSpecTag :: ChildAt( unsigned int i ) const {
	return mKids.at( i );
}

//----------------------------------------------------------------------------
// Get number of direct child nodes
//----------------------------------------------------------------------------

unsigned int XMLSpecTag :: ChildCount() const {
	return mKids.size();
}

//----------------------------------------------------------------------------
// Add an attribut spec. attributes are stored as values.
//----------------------------------------------------------------------------

void XMLSpecTag :: AddAttrib( const XMLSpecTag::Attrib & a ) {
	mAttribs.push_back( a );
}

//----------------------------------------------------------------------------
// Get attribute spec by index
//----------------------------------------------------------------------------

const XMLSpecTag::Attrib & XMLSpecTag :: AttribAt( unsigned int i ) const {
	return mAttribs.at( i );
}

//----------------------------------------------------------------------------
// How many attributes?
//----------------------------------------------------------------------------

unsigned int XMLSpecTag :: AttribCount() const {
	return mAttribs.size();
}

//----------------------------------------------------------------------------
// Set the group fields. These are specified as an comma-separated list of
// integer values.
//----------------------------------------------------------------------------

void XMLSpecTag :: SetGroup( const string & grp ) {
	if ( mGroup.size() != 0 ) {
		CSVTHROW( "Multiple groups not allowed" );
	}
	ALib::CommaList cl( grp );
	for ( unsigned int i = 0; i < cl.Size() ; i++ ) {
		mGroup.push_back( ALib::ToInteger( cl.At(i) ));
	}
}

//----------------------------------------------------------------------------
// How many fields make up group?
//----------------------------------------------------------------------------

unsigned int XMLSpecTag :: GroupCount() const {
	return mGroup.size();
}

//----------------------------------------------------------------------------
// Get individual group field value
//----------------------------------------------------------------------------

unsigned int XMLSpecTag :: GroupAt( unsigned int i ) const {
	return mGroup.at(i);
}

//----------------------------------------------------------------------------
// Dump tag node for debug
//----------------------------------------------------------------------------

void XMLSpecTag :: Dump() {
	std::cout << "---------------------------\n"
		<< (void *)this << " " << (void *) Parent() << "\n"
		<< "Tag: " << mTag << " Group: ";
	for ( unsigned int i = 0; i < mGroup.size(); i++ ) {
		std::cout << mGroup[i] << " ";
	}
	std::cout << "\n";
	for ( unsigned int i = 0; i <  AttribCount(); i++ ) {
		std::cout << " Atrrib: " << AttribAt(i).mName
				  << " Index: " << AttribAt(i).mIndex << "\n";
	}
	for ( unsigned int i = 0; i < ChildCount() ; i++ ) {
		ChildAt(i)->Dump();
	}
}

//----------------------------------------------------------------------------
// Text node specifies the field index and if this should be wrapped in
// a CDATA tag on output.
//----------------------------------------------------------------------------

XMLSpecText :: XMLSpecText( int ind, unsigned int idx, bool cdata  )
	: XMLSpecNode( ind ), mIndex( idx ), mIsCDATA( cdata ) {
}

unsigned int XMLSpecText :: Index() const {
	return mIndex;
}

void XMLSpecText :: Dump() {
	std::cout << "---------------------------\n"
			<< (void *)this << " " << (void *) Parent() << "\n"
			" Text index: " << mIndex << "\n";
}

//----------------------------------------------------------------------------
// Command help text
//----------------------------------------------------------------------------

const char * const TOXML_HELP = {
	"converts CSV data to XML\n"
	"usage: csvfix to_xml [flags] [files ...]\n"
	"where flags are:\n"
	"  -xf file \tXML specification file - produce XHTML table if omitted\n"
	"  -in ind \tindentation to use - 'tabs' means indent using tabs\n"
	"  -et \t\talways outoput explicit end-tags\n"
	"#IBL,IFN,OFL,SKIP"
};


//----------------------------------------------------------------------------
// Standard constructor & do-nothing dtor
//----------------------------------------------------------------------------

ToXMLCommand :: ToXMLCommand( const string & name,
								const string & desc )
		: Command( name, desc, TOXML_HELP ),
				mIndent(4), mUseTabs( false ), mEndTag( false ) {
	AddFlag( ALib::CommandLineFlag( FLAG_XMLSPEC, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_IND, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ENDTAG, false, 0 ) );
}

ToXMLCommand :: ~ToXMLCommand() {
}

//----------------------------------------------------------------------------
// If -xf flag used, build XML tree using config file specifuied by flag
// else just build an XHTML table.
//----------------------------------------------------------------------------

int ToXMLCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );
	IOManager io( cmd );
	CSVRow row;

	if ( mXMLSpec == "" ) {
		MakeTable( io );
	}
	else {
		std::unique_ptr <XMLSpecTag> root( ReadSpec( mXMLSpec ) );
		vector <CSVRow> input;
		while( io.ReadCSV( row ) ) {
			if ( ! Skip( row ) ) {
				input.push_back( row );
			}
		}
		InSlice all( input );
		MakeXML( io, root.get(), all, 0 );
	}
	return 0;
}

//----------------------------------------------------------------------------
// Process command line flags
//----------------------------------------------------------------------------

void ToXMLCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	mXMLSpec = cmd.GetValue( FLAG_XMLSPEC, "" );
	string in  =  cmd.GetValue( FLAG_IND, "4");
	if ( in == "tabs" ) {
		mUseTabs = true;
	}
	else {
		if ( ! ALib::IsInteger( in ) ) {
			CSVTHROW( "Indent must be integer or 'tabs' not " << in );
		}
		int inn = ALib::ToInteger( in );
		mUseTabs = false;
		if ( inn < 0 ) {
			CSVTHROW( "Indent must be 0 or greater, not " << in );
		}
		mIndent = inn;
	}
	mEndTag = cmd.HasFlag( FLAG_ENDTAG );
}


//----------------------------------------------------------------------------
// Create XHTML table from CSV input. Each CSV record becomes a <tr> and
// each field a <td> tag.
//----------------------------------------------------------------------------

void ToXMLCommand :: MakeTable( IOManager & io ) {
	io.Out() << "<table>\n";
	CSVRow row;
	while( io.ReadCSV( row ) ) {
		OuputTableRow( io.Out(), row );
	}
	io.Out() << "</table>\n";
}

void ToXMLCommand :: OuputTableRow( std::ostream & os, const CSVRow & row ) {
	os << Indent(1) << "<tr>\n";
	os << Indent(2);
	for ( unsigned int i = 0; i < row.size(); i++ ) {
		os << "<td>" << ALib::HTMLToEntity( row[i] ) << "</td>";
	}
	os << "\n" << Indent(1) << "</tr>\n";
}

//----------------------------------------------------------------------------
// Return string containing suitable characters to represent specified level
// of indentation.
//----------------------------------------------------------------------------

string ToXMLCommand :: Indent( unsigned int level ) const {
	if ( mUseTabs ) {
		return ALib::LeftPad( "", level , '\t' );
	}
	else {
		return ALib::LeftPad( "", level * mIndent, ' ' );
	}
}

//----------------------------------------------------------------------------
// Recursive function to create XML given a number of CSV records
// reprsented by the InSlice parameter. and a tag (possibly with kids). This
// works by creating smaller slice subsets for the child tags.
//----------------------------------------------------------------------------

void ToXMLCommand :: MakeXML( IOManager & io, const XMLSpecTag * t,
								const InSlice & is,
								unsigned int indent ) {
	SliceVec  myslices;
	MakeMySlices( t, is, myslices );

	for ( unsigned int i = 0; i < myslices.size(); i++ ) {
		io.Out() << Indent( indent ) << "<" << t->Name() ;
		MakeAttribs( io, t, myslices[i] );
		if ( t->ChildCount() == 0 && ! mEndTag ) {
			io.Out() << " />" << "\n";
			continue;
		}
		io.Out() << ">" << "\n";
		for ( unsigned int ci = 0; ci < t->ChildCount(); ci++ ) {
			const XMLSpecTag * cp =
					dynamic_cast <XMLSpecTag *>( t->ChildAt(ci) );
			if ( cp ) {
				MakeXML( io, cp, myslices[i], indent + 1 );
			}
			else {
				const XMLSpecText * cp =
						dynamic_cast <XMLSpecText *>( t->ChildAt(ci) );
				if ( cp ) {
					MakeText( io, cp, myslices[i], indent + 1 );
				}
			}
		}
		io.Out() << Indent( indent ) << "</" << t->Name() << ">" << "\n";
	}
}

//----------------------------------------------------------------------------
// Create an XML text element. If a cdata element was specified in the
// config file, wrap in a CDATA section else quote any XML special chars.
//----------------------------------------------------------------------------

void ToXMLCommand :: MakeText( IOManager & io, const XMLSpecText * t,
								const InSlice & is, unsigned int indent ) {

	const CSVRow & row = is[0];
	string data;
	if ( t->Index() <= row.size() ) {
		if ( t->IsCDATA() ) {
			data = row[t->Index() - 1 ];
		}
		else {
			data = ALib::HTMLToEntity( row[t->Index() - 1 ] );
		}
	}

	if ( t->IsCDATA() ) {
		io.Out() << Indent( indent ) << "<![CDATA[" << "\n";
			indent += 1;
	}

	io.Out() << Indent( indent ) << data;

	if ( t->IsCDATA() ) {
		indent -= 1;
		io.Out() << "\n" << Indent( indent ) << "]]>";
	}
	io.Out() << "\n";
}

//----------------------------------------------------------------------------
// Create attributes for a tag. Attribute values always have special
// XML characters replaced by entities.
//----------------------------------------------------------------------------

void ToXMLCommand :: MakeAttribs( IOManager & io, const XMLSpecTag * t,
											const InSlice & is ) {

	const CSVRow & row = is[0];

	for ( unsigned int i = 0; i < t->AttribCount() ; i++ ) {
		XMLSpecTag::Attrib a = t->AttribAt( i );
		io.Out() << " " << a.mName << "=\"";
		if ( a.mIndex <= row.size() ) {
			io.Out() <<  ALib::HTMLToEntity( row[a.mIndex - 1] );
		}
		io.Out() << "\"";
	}
}

//----------------------------------------------------------------------------
// Create a group key string by concatting group fields separated by NUL
// characters (which can't appear in CSV input).
//----------------------------------------------------------------------------

static string MakeGroupStr( const XMLSpecTag * t, const CSVRow & r ) {
	string gs;
	for ( unsigned int i = 0; i < t->GroupCount() ; i++ ) {
		int gp = t->GroupAt( i ) - 1;
		if ( gp < 0 || (unsigned int) gp >= r.size() ) {
			gs += "";
		}
		else {
			gs += r.at( gp );
		}
		gs += '\0';
	}
	return gs;
}

//----------------------------------------------------------------------------
// Create multiple slices for a tag by extracting from the passed in slice all
// those slices that are grouped by this tags group fields.
//----------------------------------------------------------------------------

void ToXMLCommand :: MakeMySlices( const XMLSpecTag * t,
									const InSlice & is, SliceVec & mys ){
	if ( t->GroupCount() == 0 ) {
		mys.push_back( is );
		return;
	}

	unsigned int pos = 0, begin = 0;
	string lastgroup = MakeGroupStr( t, is.at( pos ) );

	while( pos < is.size() ) {
		const CSVRow & row = is.at( pos );
		string gs = MakeGroupStr( t, row );
		if ( gs != lastgroup ) {
			InSlice a( is, begin, pos - begin );
			mys.push_back( a );
			begin = pos;
			lastgroup = gs;
		}
		pos++;
	}
	mys.push_back( InSlice( is, begin, pos - begin ));
}

//----------------------------------------------------------------------------
// Calculate indent level of line in config file
//----------------------------------------------------------------------------

static int GetIndent( const std::string & line ) {
	int i = 0;
	while( char c = ALib::Peek( line, i )) {
		if ( c >= ' ' ) {
			break;
		}
		i++;
	}
	return i;
}

//----------------------------------------------------------------------------
// Make specification node instance depending on type string
//----------------------------------------------------------------------------

XMLSpecNode * ToXMLCommand :: MakeSpecNode(
								int indent,
								const std::vector <string> &tokens ) {
	if ( tokens[0] == TAG_STR ) {
		return MakeTagSpec( indent, tokens );
	}
	else if ( tokens[0] == TEXT_STR || tokens[0] == CDATA_STR ) {
		return MakeTextSpec( indent, tokens, tokens[0] == CDATA_STR );
	}
	else {
		CSVTHROW( "Unknown type: " << tokens[0] );
	}
}

//----------------------------------------------------------------------------
// Read config file returning pointer to root tag.
//----------------------------------------------------------------------------

XMLSpecTag * ToXMLCommand :: ReadSpec( const string & file ) {

	std::ifstream ifs( file.c_str() );
	if ( ! ifs.is_open() ) {
		CSVTHROW( "Cannot open XML specification file: " << file );
	}

	string line;
	XMLSpecNode * current = 0;
	std::unique_ptr <XMLSpecTag> root;

	while( std::getline( ifs, line )) {

		int indent = GetIndent( line );
		line = ALib::Trim( line );

		if ( ALib::IsEmpty( line ) || ALib::Peek( line, 0 ) == COMMENT_CHAR ) {
			continue;
		}

		std::vector <string> tokens;
		int n = ALib::Split( line, ' ', tokens);
		if ( n < 2 ) {
			CSVTHROW( "Invalid specification: " << line );
		}

		XMLSpecNode * sn = MakeSpecNode( indent, tokens );

		if ( root.get() == 0 ) {
			if ( indent != 0 ) {
				CSVTHROW( "Invalid root indent: " << line );
			}
			root = std::unique_ptr <XMLSpecTag>(dynamic_cast<XMLSpecTag*>( sn ));
			if ( root.get() == 0 ) {
				CSVTHROW( "Root must be a tag spec at " << line );
			}
		}
		else {
			while( indent <= current->Indent() ) {
				current = current->Parent();
				if ( current == 0 ) {
					CSVTHROW( "Only one root tag allowed at "  << line );
				}
			}
			if ( XMLSpecTag * tp = dynamic_cast<XMLSpecTag*>( current ) ) {
				tp->AddChild( sn );
			}
			else {
				CSVTHROW( "Invalid XML specification at " << line );
			}
		}
		current = sn;
	}
	return root.release();
}

//----------------------------------------------------------------------------
// Helper to look at vector entry without worrying about index validity.
//----------------------------------------------------------------------------

static string  Peek( const vector<string> & v, unsigned int i ) {
	if ( i >= v.size() ) {
		return "";
	}
	else {
		return v[i];
	}
}

//----------------------------------------------------------------------------
// Helper to create attribute spec from config file data
//----------------------------------------------------------------------------

static XMLSpecTag::Attrib MakeAttrib( const vector <string> & toks,
										unsigned int pos ) {
	if ( pos + 3 > toks.size() ) {
		CSVTHROW( "Invalid attrib spec")
	}
	string name = toks[pos + 1];
	unsigned int idx = ALib::ToInteger( toks[pos + 2]);
	return XMLSpecTag::Attrib( name, idx );
}

//----------------------------------------------------------------------------
// Create tag spec from config file. Currently does not check that tag
// name is a valid XML identifier.
//----------------------------------------------------------------------------

XMLSpecNode * ToXMLCommand :: MakeTagSpec( int indent,
											const vector <string> & toks) {
	unsigned int pos = 1;
	string name = Peek( toks, pos++ );
	std::unique_ptr <XMLSpecTag> tp( new  XMLSpecTag( indent, name ) );
	while( pos < toks.size()  ) {
		if ( toks[pos] == ATTRIB_STR ) {
			tp->AddAttrib( MakeAttrib( toks, pos ) );
			pos += 3;
		}
		else if ( toks[pos] == GROUP_STR ) {
			pos++;
			if ( pos >= toks.size() ) {
				CSVTHROW( "No group fields specified");
			}
			tp->SetGroup( toks[pos++] );
		}
		else {
			CSVTHROW( "Invalid token: '" << toks[pos] << "'");
		}
	}
	return tp.release();
}

//----------------------------------------------------------------------------
// Create text spec from config file
//----------------------------------------------------------------------------

XMLSpecNode * ToXMLCommand :: MakeTextSpec( int indent,
											const vector <string> & toks,
											bool cdata) {
	unsigned int pos = 1;
	int idx = ALib::ToInteger( Peek( toks, pos++ ) );
	std::unique_ptr <XMLSpecText> tp( new  XMLSpecText( indent, idx, cdata ) );
	return tp.release();
}

//----------------------------------------------------------------------------

} // namespace
