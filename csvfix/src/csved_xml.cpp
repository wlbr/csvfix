//---------------------------------------------------------------------------
// csved_xml.cpp
//
// read xml tables for csved
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "a_xmlparser.h"
#include "a_xmltree.h"
#include "csved_cli.h"
#include "csved_xml.h"
#include "csved_strings.h"
#include <memory>

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register command
//---------------------------------------------------------------------------

static RegisterCommand <ReadXMLCommand> rc1_(
	CMD_XMLR,
	"convert XML table to CSV"
);

//---------------------------------------------------------------------------
// Standard Command ctor
//---------------------------------------------------------------------------

ReadXMLCommand :: ReadXMLCommand( const string & name,
							const string & desc )
		: Command( name, desc ) {

//	AddFlag( ALib::CommandLineFlag( FLAG_COLS, true, 1 ) );

}

//---------------------------------------------------------------------------
// Read each input stream as XML, find first <tavle> tag and convert
// it to CSV.
//---------------------------------------------------------------------------

int ReadXMLCommand :: Execute( ALib::CommandLine & cmd ) {

	IOManager io( cmd );

	for ( unsigned int i = 0; i < io.InStreamCount(); i++ ) {
		ALib::XMLTreeParser tp;
		std::auto_ptr<ALib::XMLElement>	safe_root(
					tp.ParseStream( io.In(i) )
		);

		ALib::XMLElement * root = safe_root.get();

		if ( root == 0 ) {
			CSVTHROW( "XML error '" << tp.ErrorMsg()
						<< "' in " << io.InFileName( i )
						<< " at line " << tp.ErrorLine()
			);
		}

		TableToCSV( root, io, i );
	}

	return 0;
}

//---------------------------------------------------------------------------
// Find first table in XML document rooted at e (which may or may not be
// the table) and convert to CSV.
//---------------------------------------------------------------------------

void ReadXMLCommand :: TableToCSV( const ALib::XMLElement * e,
										IOManager & io,
										unsigned int idx  ) {

	const ALib::XMLElement * table = FindTable( e );
	if ( table == 0 ) {
		CSVTHROW( "No XML table found in " <<  io.InFileName( idx ) );
	}

	for ( unsigned int i = 0; i < table->ChildCount(); i++ ) {
		const ALib::XMLElement * ce = table->ChildElement( i );
		if ( ce && ce->Name() == "tr" ) {
			WriteRow( ce, io );
		}
	}

}

//---------------------------------------------------------------------------
// find a table in tree or return 0
//---------------------------------------------------------------------------

const ALib::XMLElement *  ReadXMLCommand :: FindTable(
												const ALib::XMLElement * e ) {
	if ( e->Name() == "table" ) {
		return e;
	}

	for ( unsigned int i = 0; i < e->ChildCount(); i++ ) {
		const ALib::XMLElement * ce = e->ChildElement( i );
		const ALib::XMLElement * t = FindTable( ce );
		if ( t ) {
			return t;
		}
	}
	return 0;
}

//---------------------------------------------------------------------------
// Write an XML <tr> row out as CSV
//---------------------------------------------------------------------------

void  ReadXMLCommand :: WriteRow( const ALib::XMLElement * r,
										IOManager & io ) {
	CSVRow row;

	for ( unsigned int i = 0; i < r->ChildCount(); i++  ) {
		const ALib::XMLElement * ce = r->ChildElement( i );
		if ( ce && ce->Name() == "td" ) {
			row.push_back( TDToCSV( ce ) );
		}
	}

//	ALib::Dump( std::cout, row );
	io.WriteRow( row );
}

//---------------------------------------------------------------------------
// Write content of <td> tag as  CSV
//---------------------------------------------------------------------------

string ReadXMLCommand :: TDToCSV( const ALib::XMLElement * td ) {
	string csv;
	for ( unsigned int i = 0; i < td->ChildCount(); i++ ) {
		const ALib::XMLText * tx = td->ChildText( i );
		if ( tx ) {
			csv += tx->Text();
		}
	}

	return csv;
}

//------------------------------------------------------------------------

} // end namespace

// end

