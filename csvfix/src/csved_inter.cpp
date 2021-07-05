//---------------------------------------------------------------------------
// csved_inter.cpp
//
// interleave fields from two csv sources
//
// Copyright (C) 2010 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_inter.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register inter  command
//---------------------------------------------------------------------------

static RegisterCommand <InterCommand> rc1_(
	CMD_INTER,
	"interleave fields from two CSV sources"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const INTER_HELP = {
	"interleave fields from two CSV sources"
	"usage: csvfix inter [flags] [files ...]\n"
	"where flags are:\n"
	"  -f fields\tspecify fields to interleave\n"
	"#SMQ,SEP,IBL,IFN,OFL"
};

//------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

InterCommand :: InterCommand( const string & name,
								const string & desc )
		: Command( name, desc, INTER_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
}

//---------------------------------------------------------------------------
// Read until first stream exhausted - streams don't have to have same
// number of records. Can't use normal IOMan stuff for this as we deal
// with two input streams at same time.
//---------------------------------------------------------------------------

int InterCommand :: Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );

	IOManager io( cmd );
	if ( io.InStreamCount() != 2 ) {
		CSVTHROW( "Command requires exactly two input streams" );
	}

	std::unique_ptr<ALib::CSVStreamParser> p0( io.CreateStreamParser( 0 ) );
	std::unique_ptr<ALib::CSVStreamParser> p1( io.CreateStreamParser( 1 ) );

	CSVRow row0, row1;

	while( p0->ParseNext( row0 ) ) {
		if ( ! p1->ParseNext( row1 )) {
			row1.clear();
		}
		CSVRow out = Interleave( row0, row1 );
		io.WriteRow( out );
	}

	return 0;
}

//----------------------------------------------------------------------------
// Create field index from field spec string
//----------------------------------------------------------------------------

const char SRC0 = 'L';
const char SRC1 = 'R';

InterCommand::FieldSpec InterCommand :: MakeField( const string &  f ) const {
	if ( f.size() < 2 ) {
		CSVTHROW( "Invalid field spec " << f );
	}

	char src = std::toupper( f[0] );
	if ( src != SRC0 && src != SRC1 ) {
		CSVTHROW( "Invalid source spec in field spec " << f );
	}

	unsigned int si = src == SRC0 ? 0 : 1;
	if ( ! ALib::IsInteger( f.substr( 1 ))) {
		CSVTHROW( "Field index not integer in field " << f );
	}

	int fi = ALib::ToInteger( f.substr(1));
	if ( fi <= 0 ) {
		CSVTHROW( "Field index must be 1 or greater in field " << f );
	}

	return FieldSpec( si, fi - 1 );
}

//---------------------------------------------------------------------------
// Handle all user options with error checking
//---------------------------------------------------------------------------

void InterCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	mFields.clear();
	string fields = cmd.GetValue( FLAG_COLS, "" );
	if ( ! ALib::IsEmpty( fields ) ) {
		ALib::CommaList cl( fields );
		for ( unsigned int i = 0; i < cl.Size(); i++ ) {
			mFields.push_back( MakeField( cl.At(i) ) );
		}
	}
}

//----------------------------------------------------------------------------
// Gert field from first or second input row, depending on field spec
//----------------------------------------------------------------------------

string InterCommand :: GetField( const FieldSpec & f,
									const CSVRow & r1 ,
									const CSVRow & r2 ) const {

	const CSVRow & r = f.mSrc == 0 ? r1 : r2;
	string val = f.mField >= r.size() ? "" : r[f.mField];
	return val;
}

//----------------------------------------------------------------------------
// Given two rows, interleave using field specs.
//----------------------------------------------------------------------------

CSVRow InterCommand :: Interleave( const CSVRow & r1 ,
									const CSVRow & r2 ) const {
	if ( mFields.size() == 0 ) {
		CSVRow r( r1 );
		if ( r2.size() ) {
			ALib::Append( r, r2 );
		}
		return r;
	}
	else {
		CSVRow r;
		for ( unsigned int i = 0; i < mFields.size(); i++ ) {
			r.push_back( GetField( mFields[i], r1, r2 ));
		}
		return r;
	}
}


} // end namespace

// end

