//---------------------------------------------------------------------------
// csved_headtail.cpp
//
// head and tail commands for csvfix
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#include "csved_cli.h"
#include "csved_headtail.h"
#include "csved_except.h"
#include "csved_ioman.h"
#include "csved_strings.h"

#include <cstdlib>

using std::string;

namespace CSVED {

//----------------------------------------------------------------------------
// How many records to display by default
//----------------------------------------------------------------------------
const char * const  DEFAULT_RECORDS = "10";

//---------------------------------------------------------------------------
// Register commands
//---------------------------------------------------------------------------

static RegisterCommand <HeadCommand> rc1_(
	CMD_HEAD,
	"list first CSV records"
);

static RegisterCommand <TailCommand> rc2_(
	CMD_TAIL,
	"list last CSV records"
);

//----------------------------------------------------------------------------
// Help texts
//----------------------------------------------------------------------------

const char * const HEAD_HELP = {
	"display first records in CSV input\n"
	"usage: csvfix head [flags] [file ...]\n"
	"where flags are:\n"
	"  -n count\tdisplay first count records (default is 10)\n"
	"#ALL,SKIP"
};

const char * const TAIL_HELP = {
	"display last records in CSV input\n"
	"usage: csvfix tail [flags] [file ...]\n"
	"where flags are:\n"
	"  -n count\tdisplay last count records (default is 10)\n"
	"#ALL,SKIP"
};

//---------------------------------------------------------------------------
// Standard command ctors
//---------------------------------------------------------------------------

HeadCommand :: HeadCommand( const string & name,
							const string & desc )
	: Command( name, desc, HEAD_HELP ),
				mRecords( std::atoi( DEFAULT_RECORDS ) ){
	AddFlag( ALib::CommandLineFlag( FLAG_NUM, false, 1 ) );
}

TailCommand :: TailCommand( const string & name,
							const string & desc )
	: Command( name, desc, TAIL_HELP ),
				mRecords( std::atoi( DEFAULT_RECORDS ) ){
	AddFlag( ALib::CommandLineFlag( FLAG_NUM, false, 1 ) );
}

//----------------------------------------------------------------------------// Helper to read the number of recors to display from command line
//----------------------------------------------------------------------------
static int GetNRecords(  ALib::CommandLine & cmd  ) {

	string ns = cmd.GetValue( FLAG_NUM, DEFAULT_RECORDS );
	if ( ! ALib::IsInteger( ns ) ) {
		CSVTHROW( "Value for " << FLAG_NUM << " must be integer" );
	}
	int n = ALib::ToInteger( ns );
	if ( n <= 0 ) {
		CSVTHROW( "Value for " << FLAG_NUM << " must be greater than zero" );
	}
	return n;
}

//---------------------------------------------------------------------------
// Write the first N records to output, optionally doing skip filteering.
//---------------------------------------------------------------------------

int HeadCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	mRecords = GetNRecords( cmd );

	IOManager io( cmd );
	CSVRow row;

	unsigned int nr = 0;
	while( io.ReadCSV( row ) ) {
		if ( ! Skip( row ) ) {
			if ( ++nr > mRecords ) {
				break;
			}
			io.WriteRow( row );
		}
	}

	return 0;
}

//----------------------------------------------------------------------------// Store up input up to N records, and output on end of file
//----------------------------------------------------------------------------
int TailCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	mRecords = GetNRecords( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( ! Skip( row ) ) {
			mLastRows.push_back( row );
			if ( mLastRows.size() > mRecords ) {
				mLastRows.pop_front();
			}
		}
	}

	for( std::list <CSVRow>::iterator it = mLastRows.begin();
			it != mLastRows.end(); ++it ) {
		io.WriteRow( *it );
	}

	return 0;
}

//---------------------------------------------------------------------------

}	// end namespace

// end
