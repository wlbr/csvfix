//---------------------------------------------------------------------------
// csved_fmerge.cpp
//
// merge multiple sorted csv files
//
// Copyright (C) 20011 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_fmerge.h"
#include "csved_strings.h"

#include <string>
#include <vector>
#include <memory>

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register fmerge command
//---------------------------------------------------------------------------

static RegisterCommand <FMergeCommand> rc1_(
	CMD_FMERGE,
	"merge multiple sorted CSV files"
);

//----------------------------------------------------------------------------
// Help
//----------------------------------------------------------------------------

const char * const FMERGE_HELP = {
	"merge multiple sorted CSV files into single output\n"
	"usage: csvfix fmerge [flags] file ...\n"
	"where flags are:\n"
	"  -f fields\tfields to compare when merging (default all)\n"
	"#ALL"
};

//----------------------------------------------------------------------------
// The fmerge command
//----------------------------------------------------------------------------

FMergeCommand :: FMergeCommand( const string & name, const string & desc )
				: Command( name, desc, FMERGE_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );

}

//----------------------------------------------------------------------------
// Exec the command, merging all inputs into a single output.
//----------------------------------------------------------------------------

int FMergeCommand :: Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );
	IOManager io( cmd );
	MinFinder mf( io, mFields );

	CSVRow row;

	while( mf.FindMin( row ) ) {
		io.WriteRow( row );
	}
	return 0;
}


//----------------------------------------------------------------------------
// Get command line options
//----------------------------------------------------------------------------

void FMergeCommand :: ProcessFlags( ALib::CommandLine & cmd ) {

	if ( cmd.HasFlag( FLAG_COLS ) ) {
		ALib::CommaList cl( cmd.GetValue( FLAG_COLS ) );
		CommaListToIndex( cl, mFields );
	}
}

//----------------------------------------------------------------------------
// Create row getters for all input sources
//----------------------------------------------------------------------------

MinFinder :: MinFinder( IOManager & io, const FieldList & f ) : mFields( f ){

	for ( unsigned int i = 0; i < io.InStreamCount(); i++ ) {
		mGetters.push_back( new RowGetter( io.CreateStreamParser( i ) ));

	}
}

//----------------------------------------------------------------------------
// Delete all row getters
//----------------------------------------------------------------------------

MinFinder :: ~MinFinder() {
	for ( unsigned int i = 0; i < mGetters.size(); i++ ) {
		delete mGetters[ i ];
	}
}

//----------------------------------------------------------------------------
// Find least row and return it
//----------------------------------------------------------------------------

bool MinFinder :: FindMin( CSVRow & rmin ) {

	CSVRow row;
	int gi = -1;

	for ( unsigned int i = 0; i < mGetters.size(); i++ ) {
		bool ok = mGetters[i]->Get( row );
		if ( ok ) {
			if ( gi == -1 ||  CmpRow( row, rmin, mFields ) < 1 ) {
				rmin = row;
				gi = i;
			}
		}
	}

	if ( gi >= 0 ) {
		mGetters[gi]->ClearLatch();
	}

	return gi >= 0;
}


//----------------------------------------------------------------------------
// Getter encapsulates  astream parser and a latched input row
//----------------------------------------------------------------------------

RowGetter :: RowGetter( ALib::CSVStreamParser * p )
				: mParser( p ), mDone( false ), mHave( false ) {

}

RowGetter :: ~RowGetter() {
	delete mParser;
}


//----------------------------------------------------------------------------
// If there is anything in the latch, return that, else try to get a row
// and return that.
//----------------------------------------------------------------------------

bool RowGetter :: Get( CSVRow & row ) {
	if ( mHave ) {
		row = mLatch;
		return true;
	}
	else {
		mDone = ! mParser->ParseNext( mLatch );
		mHave = ! mDone;
		row = mLatch;
		return mHave;
	}
}

//----------------------------------------------------------------------------
// Clear the latch - used if we have located the least row.
//----------------------------------------------------------------------------

void RowGetter :: ClearLatch() {
	mHave = false;
}


} // namespace





