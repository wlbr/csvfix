//---------------------------------------------------------------------------
// csved_fromxml.cpp
//
// Convert xml file to CSV
//
// Copyright (C) 2010 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_str.h"
#include "a_collect.h"
#include "a_xmlparser.h"

#include "csved_cli.h"
#include "csved_except.h"
#include "csved_fromxml.h"
#include "csved_strings.h"
#include <memory>


using std::string;

namespace CSVED {

//----------------------------------------------------------------------------
// Register the command
//----------------------------------------------------------------------------

static RegisterCommand <FromXMLCommand> rc1_(
	CMD_FROMXML,
	"convert from XML to CSV"
);

//----------------------------------------------------------------------------
// Command help
//----------------------------------------------------------------------------

const char * const FROMXML_HELP = {
	"converts XML data to CSV records\n"
	"usage: csvfix from_xml [flags] xmlfiles\n"
	"where flags are:\n"
	"  -re path\tpath of XML tag on which to begin new CSV record\n"
	"  \t\tthe path separator is the '@' character\n"
	"  -ex path\tspecify paths to exclude from output\n"
	"  -np\t\tdo not output parent tag data\n"
	"  -nc\t\tdo not output child tag data\n"
	"  -na\t\tdo not output attribute data\n"
	"  -ip\t\tinsert tag path for record as first CSV field\n"
	"  -ml sep\tspecify multi-line text separator (default is a space)\n"
	"#SMQ.OFL"
};

//----------------------------------------------------------------------------
// Separator used to construct paths to XML tags.
//----------------------------------------------------------------------------

const char PATHSEP = '@';

//----------------------------------------------------------------------------
// The from_xml command
//----------------------------------------------------------------------------

FromXMLCommand ::	FromXMLCommand( const string & name,
								const string & desc )
		: Command( name, desc, FROMXML_HELP ),
			mFromParent( true ), mFromAttrib( true ), mFromKids( true ),
			 mInsertPath( false ), mIOMan( 0 ), mMultiLineSep( " " ) {

	AddFlag( ALib::CommandLineFlag( FLAG_XMLEREC, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_XMLNOATTR, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_XMLNOPAR, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_XMLNOKIDS, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_XMLIPATH, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_XMLMLSEP, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_XMLEXCL, false, 1 ) );

}


//----------------------------------------------------------------------------
// Execute the from_xml command. We handle the input files ourselves via
// the ALib XML library, but use the IO manager for output.
//----------------------------------------------------------------------------

const char * const STDINPUT_SPEC = "-";

int FromXMLCommand :: Execute( ALib::CommandLine & cmd )  {
	ProcessFlags( cmd );
	IOManager io( cmd, false, true );    	// don't open XML inputs
	mIOMan = & io;		// need to access the manager at various places

	if ( cmd.FileCount() == 0 ) {
		ProcessXMLFile( STDINPUT_SPEC );
	}
	else {
		for ( unsigned int i = 0; i < cmd.FileCount(); i++ ) {
			ProcessXMLFile( cmd.File( i ) );
		}
	}
	return 0;
}

//----------------------------------------------------------------------------
// Process single named XML input file. Special name "-" is std input.
//----------------------------------------------------------------------------

void FromXMLCommand :: ProcessXMLFile( const string & file ) {

	ALib::XMLTreeParser parser;
	std::unique_ptr <ALib::XMLElement> tree(
		file  == STDINPUT_SPEC
			? parser.ParseStream( std::cin )
			: parser.ParseFile( file )
	);

	// parse returns 0 on XML errors otherwise it just throws
	if ( tree.get() == 0 ) {
		CSVTHROW( "Problem parsing XML file " << parser. File()
					<< " at line " << parser.ErrorLine()
					<< ". Problem was: " << parser.ErrorMsg()  );
	}

	GetRecordsFrom( tree.get() );
}

//----------------------------------------------------------------------------
// Construct path to tag
//----------------------------------------------------------------------------

string FromXMLCommand :: MakePathTo( const ALib::XMLElement * e ) {
	string path;
	do {
		if ( path != "" ) {
			path = PATHSEP + path;
		}
		path = e->Name() + path;
		e = e->Parent();
	} while( e );
	return path;
}

//----------------------------------------------------------------------------
// Recursively get the records specified by the -re flag. As we recurse, save
// the current parent, and as we unwind, remove it. These are used to provide
// the parent data when we finally find a record we want to output.
//----------------------------------------------------------------------------

void FromXMLCommand :: GetRecordsFrom( const ALib::XMLElement * e ) {

	string path = MakePathTo( e );
	if ( IsRecordPath( path ) ) {
		mRecordPath = path;
		CSVRow row;
		if ( mInsertPath ) {
			row.push_back( path );
		}
		OutputParents( row );
		OutputRecordData( row, e, otRecord );
		mIOMan->WriteRow( row );
	}
	else {
		mParents.push_back( e );
		for ( unsigned int i = 0; i < e->ChildCount(); i++  ) {
			if ( e->ChildElement( i ) ) {
				GetRecordsFrom( e->ChildElement( i ) );
			}
		}
		mParents.pop_back();
	}
}


//----------------------------------------------------------------------------
// See if any of the path to the element the user provided exactly matches
// or is a suffix for the  path of the element we are looking at.
//----------------------------------------------------------------------------

bool FromXMLCommand :: IsRecordPath( const string & path ) {
	for ( unsigned int i = 0; i < mTagPaths.Size(); i++  ) {
		string tpath = mTagPaths.At( i );
		if ( ALib::IsSuffixed( path, tpath ) ) {
			if ( path.size() == tpath.size()
					|| path[ path.size() - (tpath.size() + 1)] == PATHSEP ) {
				return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------------
// Same for exclude path
//----------------------------------------------------------------------------

bool FromXMLCommand :: ShouldBeExcluded( const string & path ) {
	for ( unsigned int i = 0; i < mExcludePaths.Size(); i++  ) {
		string expath = mExcludePaths.At( i );
		if ( ALib::IsSuffixed( path, expath ) ) {
			if ( path.size() == expath.size()
					|| path[ path.size() - (expath.size() + 1)] == PATHSEP ) {
				return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------------
// Output the attributes of an element into a CSV row, together with any
// child elements and any text elements. THe ALib XML code
// does not concat multiline strings, so we need to handle this ourselves.
// If we are outputting the parent data we have to make sure that the
// data is only output once.
//----------------------------------------------------------------------------

void FromXMLCommand :: OutputRecordData( CSVRow & row,
											const ALib::XMLElement * e,
											OutputType ot ) {

	if ( ShouldBeExcluded( MakePathTo( e ))) {
		return;
	}

	if ( IsEmptyLeaf( e ) ) {
		row.push_back( "" );
		return;
	}

	OutputAttributes( row, e );

	string text;
	bool havetext = false;

	for ( unsigned int i = 0; i < e->ChildCount(); i++  ) {

		if ( havetext && ! e->ChildText( i ) ) {
			row.push_back( text );
			havetext = false;
			text = "";
		}

		if ( e->ChildElement(i) ) {
			if ( ot == otRecord && mFromKids ) {
				OutputRecordData( row, e->ChildElement( i ), ot );
			}
			else if ( ot == otParent && ! IsOnRecordPath( e->ChildElement(i) ) ) {
				OutputRecordData( row, e->ChildElement( i ), ot );
			}
		}
		else if ( e->ChildText( i ) ) {
			if ( text != "" ) {
				text += mMultiLineSep;
			}
			text +=  e->ChildText( i )->Text();
			havetext = true;
		}
	}

	if ( havetext ) {
		row.push_back( text );
	}
}

//----------------------------------------------------------------------------
// Output saved parents - the patent output type avoids recursing into
// the record and record-child data.
//----------------------------------------------------------------------------

void FromXMLCommand :: OutputParents( CSVRow & row ) {

	if ( ! mFromParent ) {
		return;
	}

	for ( unsigned int i = 0; i < mParents.size(); i++ ) {
		OutputRecordData( row, mParents[i], otParent );
	}
}


//----------------------------------------------------------------------------
// Is the path to the element part of the current record path?
//----------------------------------------------------------------------------

bool FromXMLCommand ::  IsOnRecordPath( const ALib::XMLElement * e ) {
	string ep = MakePathTo( e );
	return ALib::IsPrefixed( mRecordPath, ep );

}

//----------------------------------------------------------------------------
// Output tag's attributes to CSV
//----------------------------------------------------------------------------

void FromXMLCommand :: OutputAttributes( CSVRow & row,
											const ALib::XMLElement * e  ) {
	for ( unsigned int i= 0; mFromAttrib && i < e->AttrCount(); i++ ) {
		row.push_back( e->AttrValue( e->AttrName( i ) ) );
	}
}

//----------------------------------------------------------------------------
// Check if this is an empty leaf node which would normally contribute
// no data to output.
//----------------------------------------------------------------------------

bool FromXMLCommand :: IsEmptyLeaf( const ALib::XMLElement * e ) {

	if ( mFromAttrib && e->AttrCount() != 0 ) {
		return false;
	}

	for ( unsigned int i = 0; i < e->ChildCount(); i++ ) {
		if ( e->ChildElement( i ) ) {
			return false;
		}
		const ALib::XMLText * t = e->ChildText( i );
		if ( t && ! ALib::IsEmpty( t->Text() ) ) {
			return false;
		}
	}
	return true;
}

//----------------------------------------------------------------------------
// Only the -re flag is required to specify the XML entity which marks the
// start of a new CSV output record.
//----------------------------------------------------------------------------

void FromXMLCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	string paths = cmd.GetValue( FLAG_XMLEREC );
	if ( ALib::IsEmpty( paths )) {
		CSVTHROW( "Value for " << FLAG_XMLEREC << " canot be empty string" );
	}
	mTagPaths.Set( paths );

	if ( cmd.HasFlag( FLAG_XMLEXCL ) ) {
		mExcludePaths.Set( cmd.GetValue( FLAG_XMLEXCL ) );
	}

	mMultiLineSep = ALib::UnEscape( cmd.GetValue( FLAG_XMLMLSEP, " " ) );
	mFromParent = ! cmd.HasFlag( FLAG_XMLNOPAR );
	mFromAttrib = ! cmd.HasFlag( FLAG_XMLNOATTR );
	mFromKids = ! cmd.HasFlag( FLAG_XMLNOKIDS );
	mInsertPath = cmd.HasFlag( FLAG_XMLIPATH );
}

//----------------------------------------------------------------------------

}	// namespace

//----------------------------------------------------------------------------
