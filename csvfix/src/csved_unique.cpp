//---------------------------------------------------------------------------
// csved_unique.cpp
//
// remove duplicate rows
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "csved_unique.h"
#include "csved_cli.h"
#include "csved_strings.h"

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register unique command
//---------------------------------------------------------------------------

static RegisterCommand <UniqueCommand> rc1_(
	CMD_UNIQUE,
	"filter duplicate CSV records"
);

//----------------------------------------------------------------------------
// Unique command help
//----------------------------------------------------------------------------

const char * const UNIQUE_HELP = {
	"removes (or retains only) duplicate records \n"
	"usage: csvfix unique [flags] [files ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to test for uniqueness\n"
	"  -d\t\toutput only duplicate rows\n"
	"#SMQ,SEP,IBL,IFN,OFL"

};

//---------------------------------------------------------------------------
// Standard ctor
//---------------------------------------------------------------------------

UniqueCommand :: UniqueCommand( const string & name,
								const string & desc )
		: Command( name, desc, UNIQUE_HELP) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_DUPES, false, 0 ) );

}

//---------------------------------------------------------------------------
// Read inputs and remove dupes or print only dupes depending on dupes flag.
//---------------------------------------------------------------------------

int UniqueCommand :: Execute( ALib::CommandLine & cmd ) {

	ALib::CommaList cl( cmd.GetValue( FLAG_COLS, "" ) );
	CommaListToIndex( cl, mCols );
	mShowDupes = cmd.HasFlag( FLAG_DUPES );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( mShowDupes ) {
			FilterDupes( io, row );
		}
		else {
			FilterUnique( io, row );
		}
	}
	return 0;
}

//---------------------------------------------------------------------------
// Remove duplicate rows by only outputing rows with keys we don't have yet.
//---------------------------------------------------------------------------

void UniqueCommand :: FilterUnique( IOManager & io, const CSVRow & row ) {

	string key = MakeKey( row );

	MapType::iterator it = mMap.find( key );
	if ( it == mMap.end() ) {
		mMap.insert( std::make_pair( key, RowInfo( row ) ) );
		io.WriteRow( row );
	}
}

//---------------------------------------------------------------------------
// Show dupes by outputing rows we already have.
//---------------------------------------------------------------------------

void UniqueCommand :: FilterDupes( IOManager & io, const CSVRow & row ) {

	string key = MakeKey( row );

	MapType::iterator it = mMap.find( key );
	if ( it != mMap.end() ) {
		if ( it->second.mCount == 1 ) {
			io.WriteRow( it->second.mFirst );
		}
		it->second.mCount++;
		io.WriteRow( row );
	}
	else {
		mMap.insert( std::make_pair( key, RowInfo( row ) ) );
	}
}

//---------------------------------------------------------------------------
// Make key from row by concatting cols separated by null byte.
//---------------------------------------------------------------------------

string UniqueCommand :: MakeKey( const CSVRow & row ) const {

	string key;

	if ( mCols.size() == 0 ) {
		for ( unsigned int i = 0; i < row.size(); i++ ) {
			key += row[i];
			key += '\0';
		}
	}
	else {
		for ( unsigned int i = 0; i < mCols.size(); i++ ) {
			unsigned int ri = mCols[i];
			if ( ri >= row.size() ) {
				key += "";
			}
			else {
				key += row[ri];
			}
			key += '\0';
		}
	}
	return key;
}


//------------------------------------------------------------------------

} // end namespace

// end

