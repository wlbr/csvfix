//---------------------------------------------------------------------------
// csved_sort.cpp
//
// sort command for CSVED
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "a_sort.h"
#include "csved_cli.h"
#include "csved_sort.h"
#include "csved_strings.h"


using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register find command
//---------------------------------------------------------------------------

static RegisterCommand <SortCommand> rc1_(
	CMD_SORT,
	"sort CSV input on one or more fields"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const SORT_HELP = {
	"sort on specified fields\n"
	"usage: csvfix sort [flags] [files ...]\n"
	"where flags are:\n"
	"  -f  fields\tspecify fields on which to sort\n"
	"  -rh\t\tretain header in output\n"
	"\t\tfields  consist of index, and optional colon and two flags:\n"
	"\t\tN,S or I - numeric or alpha sort\n"
	"\t\tA or D   - ascending or descending sort\n"
	"\t\texample: -f 1:AN,2:DS\n"
	"#SMQ,SEP,IBL,IFN,OFL"
};

//---------------------------------------------------------------------------
// Standard Command ctor
//---------------------------------------------------------------------------

SortCommand :: SortCommand( const string & name,
							const string & desc )
		: Command( name, desc, SORT_HELP ){

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_RHEAD, false, 0 ) );

}

//----------------------------------------------------------------------------
// really need a unified header writing strategy - quick fix for now
//----------------------------------------------------------------------------

static void WriteHeader( std::ostream & os, const CSVRow & header ) {
	for ( unsigned int i = 0; i < header.size(); i++ ) {
		os << header[i];
		if ( i != header.size() - 1 ) {
			os << ",";
		}
	}
	os << "\n";
}

//---------------------------------------------------------------------------
// Build field specs from command line and the sort using alib
//---------------------------------------------------------------------------

int SortCommand :: Execute( ALib::CommandLine & cmd ) {

	bool rhead = cmd.HasFlag( FLAG_RHEAD );
	BuildFieldSpecs( cmd );

	std::vector <CSVRow> rows;

	IOManager io( cmd );
	CSVRow row, header;

	while ( io.ReadCSV( row ) ) {
		if ( rhead && header.size() == 0 ) {
			header = row;
			continue;
		}
		rows.push_back( row );
	}

	Sort( rows );

	if ( rhead ) {
		WriteHeader( io.Out(), header );
	}

	for ( unsigned int i = 0; i < rows.size(); i++ ) {
		io.WriteRow( rows[i] );
	}

	return 0;
}

//---------------------------------------------------------------------------
// do the sort via alib
//---------------------------------------------------------------------------

void SortCommand :: Sort( std::vector <CSVRow> & rows ) {

	ALib::Sorter sorter;
	for ( unsigned int i = 0; i < mFields.size(); i++ ) {
		sorter.AddField( mFields[i] );
	}
	sorter.Sort( rows );
}

//----------------------------------------------------------------------------
// Validate sort field parameters
//----------------------------------------------------------------------------

static void ValidateParams( const string & s ) {

	if ( s.size() > 2
			|| s.find_first_not_of( "ADNSI" ) != std::string::npos  ) {
		ATHROW( "Invalid field parameters: " << s)
	}

	const char * OK[] = {		// valid combinations
		"A","D","I","S","N",
		"AI","AS","AN","IA","SA", "NA",
		"DI","DS","DN","ID","SD", "ND",
		0
	};

	unsigned int i = 0;
	while( OK[i] ) {
		if ( OK[i++] == s ) {
			return;
		}
	}

	ATHROW( "Invalid field parameter combination: " << s );
}

//---------------------------------------------------------------------------
// Convert char rep field spec parameters to enum values
//---------------------------------------------------------------------------

static bool ParseFlags( const string & s,
						ALib::SortField::Direction & ad,
						ALib::SortField::CmpType & ct ) {

	ad = ALib::SortField::dirAsc;		// defaiults
	ct = ALib::SortField::ctAlpha;

	ValidateParams( s );

	if ( s.find_first_of( "D" ) != std::string::npos  ) {
		ad = ALib::SortField::dirDesc;
	}
	if ( s.find_first_of( "N" ) != std::string::npos  ) {
		ct = ALib::SortField::ctNumeric;
	}
	if ( s.find_first_of( "I" ) != std::string::npos  ) {
		ct = ALib::SortField::ctNoCase;
	}

	return true;
}

//---------------------------------------------------------------------------
// Build field specifications from command line
//---------------------------------------------------------------------------

void SortCommand :: BuildFieldSpecs( const ALib::CommandLine & cmd ) {
	if ( cmd.FlagCount( FLAG_COLS ) > 1 ) {
		CSVTHROW( "Require single " << FLAG_COLS
					<< " flag to specify sort columns" );
	}
	ALib::CommaList cl( cmd.GetValue( FLAG_COLS, "1" ) );
	if ( cl.Size() < 1 ) {
		CSVTHROW( "Fields must be specified with " << FLAG_COLS << " flag " );
	}
	for ( unsigned int i = 0; i < cl.Size(); i++ ) {
		vector <string> tmp;
		unsigned int n = ALib::Split( cl.At(i), ':', tmp );

		if ( n < 1 || n > 2 ) {
			CSVTHROW( "Invalid field specification: " << cl.At(i) );
		}

		if ( n == 1 ) {
			tmp.push_back( "AS" );		// default values
		}

		ALib::SortField::Direction direct;
		ALib::SortField::CmpType ctype;
		if ( ! ParseFlags( tmp[1], direct, ctype ) ) {
			CSVTHROW( "Invalid characters in field specification " << cl.At(i) );
		}

		if ( ! ALib::IsInteger( tmp[0] ) ) {
			CSVTHROW( "Index in field specification must be integer: " << cl.At(i) );
		}
		unsigned int idx = ALib::ToInteger( tmp[0] );
		if ( idx == 0 ) {
			CSVTHROW( "Index must be non-zero in field specification: " << cl.At(i) );
		}

		mFields.push_back( ALib::SortField( idx - 1, direct, ctype ) );
	}
}

//------------------------------------------------------------------------

} // end namespace

// end

