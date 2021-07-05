//---------------------------------------------------------------------------
// csved_put.h
//
// put fixed text or environment var into CSV
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "csved_cli.h"
#include "csved_put.h"
#include "csved_strings.h"
#include "a_date.h"
#include "a_time.h"
#include <ctime>

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register put  command
//---------------------------------------------------------------------------

static RegisterCommand <PutCommand> rc1_(
	CMD_PUT,
	"put literal or env variable into CSV output"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const PUT_HELP = {
	"puts a string literal or environment variable into CSV output\n"
	"usage: csvfix put  [flags] [file ...]\n"
	"where flags are:\n"
	"  -p pos\tposition in output(default is at end)\n"
	"  -v val\tvalue to put\n"
	"  -e env\tname of environment variable to put\n"
	"#ALL"
};

//---------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

PutCommand ::	PutCommand( const string & name,
									 const string & desc )
		: Command( name, desc, PUT_HELP ),
			mPos( -1 ), mValue( "" ) {

	AddFlag( ALib::CommandLineFlag( FLAG_POS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_VAL, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_VALENV, false, 1 ) );
}


//---------------------------------------------------------------------------
// Read input rows and put value at correct position
//---------------------------------------------------------------------------

const char * const DATETIME_STR = "@DATETIME";
const char * const DATEONLY_STR = "@DATE";
const char * const FIELDS_STR = "@COUNT";

int PutCommand ::	Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( mPos >= 0 && (unsigned int) mPos < row.size() ) {
			row.insert( row.begin() + mPos,
						mValue ==  FIELDS_STR
						? ALib::Str( row.size() )
						: mValue );
		}
		else {
			row.push_back( mValue ==  FIELDS_STR
						? ALib::Str( row.size() )
						: mValue );
		}
		io.WriteRow( row );
	}

	return 0;
}

//----------------------------------------------------------------------------
// Process command line flags. Must have eother -v or -e flags.
//----------------------------------------------------------------------------


void PutCommand :: ProcessFlags( ALib::CommandLine & cmd ) {

	NotBoth( cmd, FLAG_VAL, FLAG_VALENV, ReqOp::Required );

	if ( cmd.HasFlag( FLAG_VAL ) ) {
		mValue = cmd.GetValue( FLAG_VAL, "" );
	}
	else {
		string ev = cmd.GetValue( FLAG_VALENV, "" );
		if ( ev == DATETIME_STR ) {
			ALib::Time t( time(0) );
			mValue = t.TimeStampStr();
		}
		else if ( ev == DATEONLY_STR ) {
			ALib::Date date;
			mValue = date.Str();
		}
		else if ( ev == FIELDS_STR ) {
			mValue = FIELDS_STR;
		}
		else {
			mValue = ALib::GetEnv( ev );
		}
	}

	string pos = cmd.GetValue( FLAG_POS, "" );
	if ( pos == "" ) {
		mPos = -1;
	}
	else {
		if ( ALib::IsInteger( pos ) ) {
			mPos = ALib::ToInteger( pos ) - 1;
			if ( mPos < 0 ) {
				CSVTHROW( "Invalid  position value: " << pos );
			}
		}
		else {
			CSVTHROW( "Position must be non-zero integer");
		}
	}
}

//------------------------------------------------------------------------

} // namespace

// end

