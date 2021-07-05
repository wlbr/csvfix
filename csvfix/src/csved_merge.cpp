//---------------------------------------------------------------------------
// csved_merge.cpp
//
// merge csv field data
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_merge.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register find command
//---------------------------------------------------------------------------

static RegisterCommand <MergeCommand> rc1_(
	CMD_MERGE,
	"merge fields"
);

//----------------------------------------------------------------------------
// merge command help
//----------------------------------------------------------------------------

const char * const MERGE_HELP = {
	"merges multiple CSV fields into a single field\n"
	"usage: csvfix merge [flags] [files ...]\n"
	"where flags are:\n"
	"  -f fields\tspecify fields to merge (default is to merge all fields)\n"
	"  -s sep\tcharacter(s) to use as separator\n"
	"  -p pos\tposition to insert merged field in output\n"
	"  -k\t\tretain original merged fields in output\n"
	"#SMQ,SEP,IBL,IFN,OFL,SKIP,PASS"
};

//---------------------------------------------------------------------------
// standard command constructor
//---------------------------------------------------------------------------

MergeCommand ::	MergeCommand( const string & name,
								const string & desc )
		: Command( name, desc, MERGE_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_POS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_SEP, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_KEEP, false, 0 ) );
}

//---------------------------------------------------------------------------
// Process the falgs then do merge on the inputs
//---------------------------------------------------------------------------

int MergeCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}
		if ( ! Pass( row ) ) {
			DoMerge( row );
		}
		io.WriteRow( row );
	}

	return 0;
}


//---------------------------------------------------------------------------
// Merge fields specified by the fields flag, discarding the merged rows
// unless the -k flag is specified. Place the merged data at the position
// specified by the -p flag.
//
// Now treat an empty field list (i.e. no -f flag) as meaning all fields.
//---------------------------------------------------------------------------

void MergeCommand :: DoMerge( CSVRow & row ) {

	string merged;

	if ( mCols.size() != 0 ) {
		for ( unsigned int i = 0; i < mCols.size(); i++ ) {
			unsigned int ci = mCols[ i ];
			if ( ci < row.size() ) {
				string s = row[ ci ];
				if ( i != mCols.size() - 1 ) {
					s += mSep;
				}
				merged += s;
			}
		}
	}
	else {
		for ( unsigned int i = 0; i < row.size(); i++ ) {
			if ( ! merged.empty() ) {
				merged += mSep;
			}
			merged += row[i];
		}
	}

	BuildNewRow(row, merged );
}

//----------------------------------------------------------------------------
// Create a new row with the merged value positioned correctly and then
// swap it with the existing row to save a copy.
//----------------------------------------------------------------------------

void MergeCommand :: BuildNewRow( CSVRow & row, const string & merged ) {
	CSVRow newrow;
	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( mPos == i ) {
			newrow.push_back( merged );
		}
		if ( mKeep || (mCols.size() != 0 && ! ALib::Contains( mCols, i )) ) {
			newrow.push_back( row[ i ] );
		}
	}

	if ( mPos >= row.size() ) {
		newrow.push_back( merged );
	}

	row.swap( newrow );
}

//----------------------------------------------------------------------------
// Expand special characters in separator. These are tabs, newlines and
// carriage returns and are encoded as for C strings.
//----------------------------------------------------------------------------

static string ExpandSep( const string & sep ) {
	string s;
	for ( unsigned int i = 0; i < sep.size() ; i++ ) {
		if ( sep[i] == '\\' ) {
			i++;
			if ( i == sep.size() ) {
				CSVTHROW( "Invalid escape at end of separator: " << sep );
			}
			else if ( sep[i] == 't' ) {
				s += '\t';
			}
			else if ( sep[i] == 'n') {
				s += '\n';
			}
			else if ( sep[i] == 'r') {
				s += '\r';
			}
			else if ( sep[i] == '\\' ) {
				s += '\\';
			}
			else {
				CSVTHROW( "Invalid special character: \\" << sep[i] );
			}
		}
		else {
			s += sep[i];
		}
	}

	return s;
}

//---------------------------------------------------------------------------
// Process user flags. If no -f flag is specified, this means merge all
// fields into one.
//---------------------------------------------------------------------------

void MergeCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {


	if ( cmd.HasFlag( FLAG_COLS ) ) {
		string s = cmd.GetValue( FLAG_COLS );
		ALib::CommaList cl( s );
		CommaListToIndex( cl, mCols );
		if ( mCols.size() <= 1 ) {
			CSVTHROW( "Need to specify two or more fields with "
						<< FLAG_COLS << " flag" );
		}
	}

	mSep = ExpandSep( cmd.GetValue( FLAG_SEP, " ") );
	string defmpos = mCols.size() ? ALib::Str( mCols[0] + 1 ) : "1";
	string s = cmd.GetValue( FLAG_POS, defmpos );

	if (  ! ALib::IsInteger ( s ) ) {
		CSVTHROW( "Position specified by " << FLAG_POS << " must be integer" );
	}

	int pos = ALib::ToInteger( s ) - 1;
	if ( pos  < 0 )  {
		CSVTHROW( "Position specified by " << FLAG_POS
						<< " must be greater than zero" );
	}

	mPos = pos;
	mKeep = cmd.HasFlag( FLAG_KEEP );
}


//------------------------------------------------------------------------

} // end namespace

// end

