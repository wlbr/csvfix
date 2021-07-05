//---------------------------------------------------------------------------
// csved_timestamp.cpp
//
// add timestamps to CSV input
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_timestamp.h"
#include "csved_strings.h"

#include <iomanip>
#include <sstream>
#include <string>
using std::string;

//----------------------------------------------------------------------------

namespace CSVED {

//---------------------------------------------------------------------------
// Register timestamp command
//---------------------------------------------------------------------------

static RegisterCommand <TimestampCommand> rc1_(
	CMD_TSTAMP,
	"add timestamp to CSV data"
);

//----------------------------------------------------------------------------
// Help
//----------------------------------------------------------------------------

const char * const TSTAMP_HELP = {
	"adds a timestamp field to CSV input data\n"
	"usage: csvfix timestam  [flags] [file ...]\n"
	"where flags are:\n"
	"  -d \t\tdisplay only date part of timestamp\n"
	"  -t \t\tdisplay only time part of timestamp\n"
	"  -n \t\tdisplay timestamp as numeric value with no separators\n"
	"  -rt \t\tupdate stamp in real time as records are written\n"
	"#ALL,SKIP"
};

//----------------------------------------------------------------------------
// The timestamp command
//----------------------------------------------------------------------------

TimestampCommand :: TimestampCommand( const string & name, const string & desc )
		: Command( name, desc, TSTAMP_HELP ),
		  mRealTime( false ),
		  mShowDate( true ),
		  mShowTime( true ),
		  mNumericStamp( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_RTIME, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_TONLY, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_DONLY, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_NUM, false, 0 ) );
}


//----------------------------------------------------------------------------
// Format timestamp depending on user options. std::put_time not yet
// available for gcc, so we do the formatting ourselves.
//----------------------------------------------------------------------------

string TimestampCommand :: FormatStamp( std::time_t t ) const {

    std::tm tm = * std::localtime( & t );
    std::ostringstream os;
	string ysep = mNumericStamp ? "" : "-";
	string tsep = mNumericStamp ? "" : ":";

	if ( mShowDate ) {
		os << tm.tm_year + 1900 << ysep
			<< std::setw(2) << std::setfill( '0' ) <<  tm.tm_mon + 1 << ysep
			<< std::setw(2) << std::setfill( '0' ) <<  tm.tm_mday;
	}

	if ( mShowDate && mShowTime  && ! mNumericStamp ) {
		os << " ";
	}

	if ( mShowTime ) {
		os << std::setw(2) << std::setfill( '0' ) <<  tm.tm_hour << tsep
			<< std::setw(2) << std::setfill( '0' ) <<  tm.tm_min << tsep
			<< std::setw(2) << std::setfill( '0' ) <<  tm.tm_sec;
	}

	return os.str();
}

//----------------------------------------------------------------------------
// If not using the real-time flag, only get time stamp once and stamp
// all records with the same value. Otherwise, the stamp is updated
// on each record read.
//----------------------------------------------------------------------------

int TimestampCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;

	string stamp = FormatStamp( std::time(0) );

	while( io.ReadCSV( row ) ) {

		if ( Skip( row ) ) {
			continue;
		}

		if ( mRealTime ) {
			stamp = FormatStamp( std::time(0) );
		}
		CSVRow out;
		out.push_back( stamp );
		ALib::Append( out, row );
		io.WriteRow( out );
	}

	return 0;
}


//----------------------------------------------------------------------------
// Handle command line options
//----------------------------------------------------------------------------

void TimestampCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {
	NotBoth( cmd, FLAG_DONLY, FLAG_TONLY );
	mShowDate  = ! cmd.HasFlag( FLAG_TONLY );
	mShowTime = ! cmd.HasFlag( FLAG_DONLY );
	mRealTime = cmd.HasFlag( FLAG_RTIME );
	mNumericStamp = cmd.HasFlag( FLAG_NUM );
}


//----------------------------------------------------------------------------


} // namespace

