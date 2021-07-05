//---------------------------------------------------------------------------
// csved_fixed.cpp
//
// read/write fixed format data
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_fixed.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register fixed commands
//---------------------------------------------------------------------------

static RegisterCommand <ReadFixedCommand> rc1_(
	CMD_FIXREAD,
	"convert fixed format data to CSV"
);

static RegisterCommand <WriteFixedCommand> rc2_(
	CMD_FIXWRITE,
	"convert CSV to fixed format"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const RFIX_HELP = {
	"read fixed-format records and convert to CSV data\n"
	"usage: csvfix read_fixed  [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tspecify fields to extract from fixed-format\n"
	"  -k\t\tretain trailing spaces on output\n"
	"#AIBL,SMQ,OFL"
};

const char * const WFIX_HELP = {
	"convert input CSV data to fixed format (non-CSV) output\n"
	"usage: csvfix read_fixed  [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tspecify fields and widths to to output as fixed-format\n"
	"#AIBL.IFN,OFL,SEP,SKIP"
};

//
//---------------------------------------------------------------------------
// Base class manages field specs
//---------------------------------------------------------------------------

FixedCommand :: FixedCommand( const string & name,
								const string & desc, const string & help )
		: Command( name, desc, help ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, true, 1 ) );
}

//---------------------------------------------------------------------------
// Field specs have common  format (but different meanings) in derived
// classes. Here we build the specs from list of colon-separated pairs.
// Example of a field spec is  '3:7,1:1,2:4'
//---------------------------------------------------------------------------

void FixedCommand :: BuildFields( const ALib::CommandLine & cmd ) {

	mFields.clear();

	ALib::CommaList cl( cmd.GetValue( FLAG_COLS ) );  // chop into pairs

	if ( cl.Size() == 0 )  {
		CSVTHROW( "Need fields specified with " << FLAG_COLS << " flag" );
	}

	for ( unsigned int i = 0; i < cl.Size(); i++ ) {

		vector <string> tmp;
		if ( ALib::Split( cl.At(i), ':', tmp ) != 2 ) {
			CSVTHROW( "Invalid field specification: " << cl.At(i) );
		}

		if ( ! ALib::IsInteger( tmp[0] ) || ! ALib::IsInteger( tmp[1] ) ) {
 			CSVTHROW( "Invalid field specification: " << cl.At(i) );
		}

		unsigned int f1 = ALib::ToInteger( tmp[0] );
		unsigned int f2 = ALib::ToInteger( tmp[1] );
		if ( f1 == 0 || f2 == 0 ) {
			CSVTHROW( "Invalid field specification: " << cl.At(i) );
		}

		mFields.push_back( std::make_pair( f1, f2 ) );
	}
}

//------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

ReadFixedCommand ::	ReadFixedCommand( const string & name,
								const string & desc )
		: FixedCommand( name, desc, RFIX_HELP ), mTrim( true ) {

	AddFlag( ALib::CommandLineFlag( FLAG_KEEP, false, 0 ) );

}

//---------------------------------------------------------------------------
// Raed all inputs as text and thenn chop up into fields which are output
// in CSV format
//---------------------------------------------------------------------------

int ReadFixedCommand :: Execute( ALib::CommandLine & cmd ) {

	mTrim = ! cmd.HasFlag( FLAG_KEEP );

	BuildFields( cmd );

	IOManager io( cmd );
	string line;

	CSVRow row;
	while( io.ReadLine( line ) ) {
		MakeRow( line, row );
		io.WriteRow( row );
	}

	return 0;
}

//---------------------------------------------------------------------------
// Make the CSV fields using field specs
//---------------------------------------------------------------------------

void ReadFixedCommand :: MakeRow( const string & line, CSVRow & row ) {
	row.clear();
	unsigned int len = line.size();
	for( unsigned int i = 0; i < mFields.size(); i++ ) {
		if ( mFields[i].first > len ) {
			row.push_back( "" );
		}
		else {
			string val = line.substr( mFields[i].first - 1, mFields[i].second );
			row.push_back( mTrim ? ALib::RTrim( val ) : val );
		}
	}
}

//---------------------------------------------------------------------------
// Standard ctor
//---------------------------------------------------------------------------

WriteFixedCommand :: WriteFixedCommand( const string & name,
										const string & desc )
		: FixedCommand( name, desc, WFIX_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_RULER, false, 0 ) );

}

//---------------------------------------------------------------------------
// Create 80-char width ruler
//---------------------------------------------------------------------------

static string Ruler() {
	string r;
	for ( unsigned int i = 0; i < 8; i++ ) {
		r += "123456789 ";
	}
	return r;
}

//---------------------------------------------------------------------------
// Read CSV and write as fixed format text
//---------------------------------------------------------------------------

int WriteFixedCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	BuildFields( cmd );

	IOManager io( cmd );

	if ( cmd.HasFlag( FLAG_RULER ) ) {
		io.Out() << Ruler() << "\n";
	}

	CSVRow row;
	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}
		string line = MakeFixedOutput( row );
		io.Out() << line << "\n";
	}

	return 0;
}

//---------------------------------------------------------------------------
// Produce single line of fixed format output specified by field specs
// in the form:
//
//	 field:width,field:width,...
//
// Fields may appear zero or more times. If a field is missing from
// input it is assumed to contain empty string.
//---------------------------------------------------------------------------

string WriteFixedCommand :: MakeFixedOutput( const CSVRow & row ) {

	string s;

	for ( unsigned int i = 0; i < mFields.size(); i++ ) {
		string val = mFields[i].first > row.size()
						? ""
						: row[ mFields[i].first - 1 ];
		unsigned int width = mFields[i].second;

		s += ALib::RightPad( val, width );
	}

	return s;
}


//------------------------------------------------------------------------

} // end namespace

// end

