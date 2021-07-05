//---------------------------------------------------------------------------
// csved_seq.cpp
//
//  add sequence numbers to input
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "csved_cli.h"
#include "csved_seq.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//----------------------------------------------------------------------------
// Character to use to represent seq number insertion in mask
//----------------------------------------------------------------------------

const char MASK_CHAR = '@';

//---------------------------------------------------------------------------
// Register sequence command
//---------------------------------------------------------------------------

static RegisterCommand <SeqCommand> rc1_(
	CMD_SEQ,
	"add sequence numbers"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const SEQ_HELP = {
	"add sequence numbers to output\n"
	"usage: csvfix sequence [flags] [files ...]\n"
	"where flags are:\n"
	"  -n start\tspecify sequence start value\n"
	"  -i inc\tincrement between sequence numbers\n"
	"  -d dec\tdecrement between sequence numbers (mutually exclusive with -i)\n"
	"  -p pad\tprovide width (with left zero padding) of sequence numbers\n"
	"  -f pos\tspecify index of field to place sequence numberr at\n"
	"  -m mask\tspecify mask to apply to sequence number\n"
	"#SMQ,SEP,IBL,IFN,OFL,SKIP,PASS"
};

//------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

SeqCommand ::SeqCommand( const string & name,
								const string & desc )
		: Command( name, desc, SEQ_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_NUM, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_PAD, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_INC, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_MASK, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_DECR, false, 1 ) );

}

//---------------------------------------------------------------------------
// Get user specifie doptions and then read input, inserting sequence
// number at specified position
//---------------------------------------------------------------------------

int SeqCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if( Skip( row ) ) {
			continue;
		}
		if ( ! Pass( row ) ) {
			AddSeq( row );
		}
		io.WriteRow( row );
	}

	return 0;
}

//---------------------------------------------------------------------------
// Handle all user options with error checking
//---------------------------------------------------------------------------

void SeqCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	NotBoth( cmd, FLAG_INC, FLAG_DECR );

	string sn = cmd.GetValue( FLAG_COLS, "1" );
	if ( sn != "" && ! ALib::IsInteger( sn) ) {
		CSVTHROW( "Field number specified by " << FLAG_COLS
				<< " must be integer" );
	}

	if ( ALib::ToInteger( sn ) - 1  < 0 ) {
		CSVTHROW( "Field number specified by " << FLAG_COLS
				<< " must be 1 or greater" );
	}
	mCol = ALib::ToInteger( sn ) - 1;

	sn = cmd.GetValue( FLAG_NUM, "1" );
	if ( sn != "" && ! ALib::IsInteger( sn) ) {
		CSVTHROW( "Start number specified by " << FLAG_NUM
				<< " must be integer" );
	}
	mSeqNo = ALib::ToInteger( sn );

	sn = cmd.GetValue( FLAG_PAD, "0" );
	if ( sn != "" && ! ALib::IsInteger( sn) ) {
		CSVTHROW( "Padding specified by " << FLAG_PAD
				<< " must be integer" );
	}
	if ( ALib::ToInteger( sn ) < 0 ) {
		CSVTHROW( "Padding specified by " << FLAG_PAD
				<< " must be 0 or greater" );
	}
	mPad = ALib::ToInteger( sn );

	sn = cmd.GetValue( FLAG_INC, "1" );
	if ( sn != "" && ! ALib::IsInteger( sn) ) {
		CSVTHROW( "Increment specified by " << FLAG_INC
				<< " must be integer" );
	}

	// fix for bug in command line parser that can't handle negative
	// numbers as parameters for switches
	if ( cmd.HasFlag( FLAG_DECR ) ) {
		sn = cmd.GetValue( FLAG_DECR );
		if ( sn == "" || ! ALib::IsInteger(( sn ) )) {
			CSVTHROW( "Invalid decrement value: " << sn );
		}
		else {
			sn = "-" + sn;
		}
	}

	if ( (mInc = ALib::ToInteger( sn )) == 0 ) {
		CSVTHROW( "Increment/decrement must not be zero" );
	}

	if ( cmd.HasFlag( FLAG_MASK ) ) {
		mMask = cmd.GetValue( FLAG_MASK );
		if ( mMask.find( MASK_CHAR )  == std::string::npos ) {
			CSVTHROW( "Invalid sequence number mask" );
		}
	}
	else {
		mMask = "";
	}
}

//----------------------------------------------------------------------------
// Insert sequence number into mask. already checks that the mask contains
// am insertion marker.
//----------------------------------------------------------------------------

string SeqCommand :: MaskSeq( const std::string & sn ) const {
	string m;
	std::string::size_type  p = mMask.find( MASK_CHAR );
	m = mMask.substr( 0, p ) + sn + mMask.substr( p + 1 );
	return m;
}

//---------------------------------------------------------------------------
// Put sequence no. in position specified by fields flag. If the position
// does not exist, append to end of row
//---------------------------------------------------------------------------

void SeqCommand :: AddSeq( CSVRow & row ) {

	string sn = mSeqNo >= 0 && mPad > 0
				? ALib::ZeroPad( mSeqNo, mPad )
				: ALib::Str( mSeqNo );

	if ( mMask != "" ) {
		sn = MaskSeq( sn );
	}

	mSeqNo += mInc;

	CSVRow newrow;
	bool added = false;

	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( i == mCol ) {
			newrow.push_back( sn );
			added = true;
		}
		newrow.push_back( row[ i ] );
	}

	if ( ! added ) {
		newrow.push_back( sn );
	}

	newrow.swap( row );
}


} // end namespace

// end

