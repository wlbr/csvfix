//---------------------------------------------------------------------------
// csved_block.cpp
//
// perform actions on blocks of CSV records
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_block.h"
#include "csved_evalvars.h"
#include "csved_strings.h"
#include <string>
using std::string;

namespace CSVED {


//---------------------------------------------------------------------------
// Register block command
//---------------------------------------------------------------------------

static RegisterCommand <BlockCommand> rc1_(
	CMD_BLOCK,
	"perform actions on blocks of CSV records"
);


//----------------------------------------------------------------------------
// Help
//----------------------------------------------------------------------------

const char * const BLOCK_HELP = {
	"perform actions on blocks of CSV records\n"
	"usage: csvfix block  [flags] [file ...]\n"
	"where flags are:\n"
	"  -be expr\texpression marking start of block\n"
	"  -ee expr\texpression marking end of block\n"
	"  -r\t\tremove block from output\n"
	"  -k\t\tkeep block in output\n"
	"  -m mark\tmark block and non-block records\n"
	"  -x\t\texclude begin/end records from block\n"
	"#ALL"
};

//----------------------------------------------------------------------------
// The block command
//----------------------------------------------------------------------------

BlockCommand :: BlockCommand( const string & name, const string & desc )
				: Command( name, desc, BLOCK_HELP ) ,
					mAction( None ), mExclusive( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_BEXPR, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_EEXPR, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ACTKEEP, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ACTREM, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ACTMARK, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_BLKEXC, false, 0 ) );
}


//----------------------------------------------------------------------------
// Are we inside or outside a block?
//----------------------------------------------------------------------------

enum class InOut { Outside, Inside };

//----------------------------------------------------------------------------
// Read rows and flip state depending on the return value of the begin/end
// expressions. Need to take into account if we are in exclude mode, in
// which case the begin and end marker rows are excluded from blocks.
//----------------------------------------------------------------------------

int BlockCommand :: Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );
	IOManager io( cmd );
	CSVRow row;
	InOut state = InOut::Outside;
	bool block = true;

	while( io.ReadCSV( row ) ) {
		if ( state == InOut::Outside ) {
			AddVars( mBeginEx, io, row );
			if ( AtBeginBlock() ) {
				block = ! mExclusive;
				state = InOut::Inside;
			}
			else {
				block = false;
			}
		}
		else if ( state == InOut::Inside ) {
			AddVars( mEndEx, io, row );
			if ( AtEndBlock() ) {
				block = ! mExclusive;
				state = InOut::Outside;
			}
			else {
				block = true;
			}
		}
		else {
			CSVTHROW( "Bad state" );
		}

		//std::cout << "Block is " << block << std::endl;
		if ( mAction == Mark ) {
			CSVRow tmp;
			tmp.push_back( block ? mBlockMark : mNotMark );
			tmp.insert( tmp.end(), row.begin(), row.end() );
			io.WriteRow( tmp );
		}
		else if ( (block && mAction == Keep) || (!block && mAction == Remove) ) {
			io.WriteRow( row );
		}
	}
	return 0;
}

//----------------------------------------------------------------------------
// Are we at the rows marking begin/end of blocks?
//----------------------------------------------------------------------------

bool BlockCommand :: AtEndBlock()  {
	return ALib::Expression::ToBool( mEndEx.Evaluate() );
}

bool BlockCommand :: AtBeginBlock()  {
	return ALib::Expression::ToBool( mBeginEx.Evaluate() );
}

//----------------------------------------------------------------------------
// Helper to get begin/end expression
//----------------------------------------------------------------------------

static void GetBEExpr( const ALib::CommandLine & cmd, const string & which,
							ALib::Expression & expr ) {

	string exs = cmd.GetValue( which );
	string emsg = expr.Compile( exs );
	if ( emsg != "" ) {
		CSVTHROW( "Invalid expression for " << which );
	}
}


//----------------------------------------------------------------------------
// Get command line options and report any problems  with them.
//----------------------------------------------------------------------------

void BlockCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	GetBEExpr( cmd, FLAG_BEXPR, mBeginEx );
	GetBEExpr( cmd, FLAG_EEXPR, mEndEx );

	int actions = 0;
	if ( cmd.HasFlag( FLAG_ACTREM ) ) {
		mAction = Remove;
		actions++;
	}
	if ( cmd.HasFlag( FLAG_ACTKEEP ) ) {
		mAction = Keep;
		actions++;

	}
	if ( cmd.HasFlag( FLAG_ACTMARK ) ) {
		mAction = Mark;
		actions++;
		ALib::CommaList cl( cmd.GetValue( FLAG_ACTMARK, "" ) );
		if ( cl.Size() == 0 ) {
			CSVTHROW( "Need value for " << FLAG_ACTMARK );
		}
		else if ( cl.Size() == 1 ) {
			mBlockMark = cl.At(0);
			mNotMark = "";
		}
		else if ( cl.Size() == 2 ) {
			mBlockMark = cl.At(0);
			mNotMark = cl.At(1);
		}
		else {
			CSVTHROW( "Invalid mark string for " << FLAG_ACTMARK );
		}
	}
	if ( actions != 1 ) {
		CSVTHROW( "Need only one of "
					<< FLAG_ACTKEEP << ", "
					<< FLAG_ACTREM <<" or "
					<< FLAG_ACTMARK );
	}

	mExclusive = cmd.HasFlag( FLAG_BLKEXC );
}


//----------------------------------------------------------------------------

} // namespace





