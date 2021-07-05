//---------------------------------------------------------------------------
// csved_shuffle.cpp
//
// randomly shuffle csv rows and fields
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_rand.h"
#include "csved_cli.h"
#include "csved_shuffle.h"
#include "csved_strings.h"
#include <ctime>
#include <algorithm>

using std::string;
using std::vector;

namespace CSVED {

//----------------------------------------------------------------------------
// Register shuffle command
//----------------------------------------------------------------------------

static RegisterCommand <ShuffleCommand> rc1_(
	CMD_SHUFFLE,
	"randomly shuffle rows or fields"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const SHUFFLE_HELP = {
	"shuffles input CSV data to produce randomised output\n"
	"usage: csvfix shuffle [flags] [files ...]\n"
	"where flags are:\n"
	"  -n count\tonly output the first 'count' rows after shuffle\n"
	"  -rs seed\tspecify seed to use for random number generator\n"
	"  -f fields\tshuffle specified fields instead of rows\n"
	"#SMQ,SEP,IBL,IFN,OFL"
};

//----------------------------------------------------------------------------
// Standard command ctor
//----------------------------------------------------------------------------

ShuffleCommand ::ShuffleCommand( const string & name,
								const string & desc )
		: Command( name, desc, SHUFFLE_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_NUM, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_RSEED, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
}

//----------------------------------------------------------------------------
// need to read all rows into memory so we can shuffle them
//----------------------------------------------------------------------------

int ShuffleCommand :: Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );
	mRows.clear();
	IOManager io( cmd );
	CSVRow row;

	if ( mFields.size() == 0 ) {
		while( io.ReadCSV( row ) ) {
			mRows.push_back( row );
		}
		Shuffle( io );
	}
	else {
		while( io.ReadCSV( row ) ) {
			ShuffleFields( row );
			io.WriteRow( row );
		}
	}
	return 0;
}

//----------------------------------------------------------------------------
// Handle all user options with error checking
//----------------------------------------------------------------------------

void ShuffleCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	string sn = cmd.GetValue( FLAG_NUM, ALib::Str(INT_MAX -1) );
	if ( ! ALib::IsInteger( sn )) {
		CSVTHROW( "Value for " << FLAG_NUM << " must be integeer" );
	}

	if ( ALib::ToInteger( sn ) < 0 ) {
		CSVTHROW( "Value for " << FLAG_NUM << " must be zero or greater" );
	}
	mCount = ALib::ToInteger( sn );

	string sr = cmd.GetValue( FLAG_RSEED, "-1" );
	if ( ! ALib::IsInteger( sr ) ) {
		CSVTHROW( "Value for " << FLAG_RSEED << " must be integer" );
	}
	int n = ALib::ToInteger( sr );
	mSeed = n < 0 ? std::time(0) : n;
	if ( cmd.HasFlag( FLAG_COLS ) ) {
		ALib::CommaList cl( cmd.GetValue( FLAG_COLS ) );
		CommaListToIndex( cl, mFields );
	}
}

//----------------------------------------------------------------------------
// Shuffle fields specified by the -f option
//----------------------------------------------------------------------------

void ShuffleCommand ::ShuffleFields( CSVRow & row  ) {
	std::vector <string> f;
	for( unsigned int i = 0; i < mFields.size(); i++ ) {
		unsigned int fi = mFields[i];
		if ( fi >= row.size() ) {
			CSVTHROW( "Invalid field index: " << fi + 1 );
		}
		else {
			f.push_back( row[fi] );
		}
	}
	std::random_shuffle( f.begin(), f.end() );
	unsigned int ri = 0;
	for( unsigned int i = 0; i < mFields.size(); i++ ) {
		unsigned int fi = mFields[i];
		row[fi] = f[ri++];
	}
}

//----------------------------------------------------------------------------
// shuffle rows and output via io manager
//----------------------------------------------------------------------------

void ShuffleCommand :: Shuffle( IOManager & io ) {
	ALib::RandGen rg( mSeed );
	int nout = mCount;
	int last = mRows.size();
	for ( unsigned int i = 0; i < mRows.size() && nout-- > 0; i++ ) {
		int pos = rg.NextInt( 0, last );
		io.WriteRow( mRows[pos] );
		mRows[pos].swap( mRows[--last] );
	}
}

//----------------------------------------------------------------------------

} // end namespace

// end

