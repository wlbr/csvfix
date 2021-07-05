//---------------------------------------------------------------------------
// csved_readmulti.cpp
//
// convert multi line input to csv
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "csved_cli.h"
#include "csved_readmulti.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register read_multi command
//---------------------------------------------------------------------------

static RegisterCommand <ReadMultiCommand> rc1_(
	CMD_READMUL,
	"read multi-line data"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const RMUL_HELP = {
	"read multi-line data and convert to CSV\n"
	"usage: csvfix file_info  [flags] [file ...]\n"
	"where flags are:\n"
	"  -n lines\tspecify number of lines in each multi-line record\n"
	"  -s sep\tspecify separator between records\n"
	"#SMQ,OFL"
};

//---------------------------------------------------------------------------
// standard command constructor
//---------------------------------------------------------------------------

ReadMultiCommand :: ReadMultiCommand( const string & name,
								const string & desc )
		: Command( name, desc, RMUL_HELP ), mNumLines( 0 ) {

	AddFlag( ALib::CommandLineFlag( FLAG_NUM, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_SEP, false, 1 ) );
}

//---------------------------------------------------------------------------
// Process flags then read rows
//---------------------------------------------------------------------------

int ReadMultiCommand :: Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;
	string line;
	unsigned int nread = 0;

	while( io.ReadLine( line ) ) {
		nread++;
		if ( mNumLines != 0 ) {
			row.push_back( line );
			if ( nread == mNumLines ) {
				io.WriteRow( row );
				row.clear();
				nread = 0;
			}
		}
		else {
			if ( line == mSep ) {
				io.WriteRow( row );
				row.clear();
			}
			else {
				row.push_back( line );
			}
		}
	}

	if ( row.size() > 0 ) {
		io.WriteRow( row );
	}

	return 0;
}

//----------------------------------------------------------------------------
// Need one of -s or -n flags. Note the number of lines that can be read
// is limited to MAX_LINES so mistaken huge numbers are caught.
//----------------------------------------------------------------------------

void ReadMultiCommand :: ProcessFlags( ALib::CommandLine & cmd ) {

	const unsigned int MAX_LINES = 200;	    // max allowed multi lines

	NotBoth( cmd, FLAG_SEP, FLAG_NUM, ReqOp::Required );

	if ( cmd.HasFlag( FLAG_NUM ) ) {
		string ns = cmd.GetValue( FLAG_NUM );
		if ( ! ALib::IsInteger( ns )
			|| ( mNumLines = ALib::ToInteger( ns ) ) < 1
			|| mNumLines > MAX_LINES  ) {
			CSVTHROW( "Invalid number of lines: " << mNumLines  );
		}
	}
	else {
		mSep = cmd.GetValue( FLAG_SEP );
		if ( mSep == "" ) {
			CSVTHROW( "Empty separator" );
		}
	}
}

//----------------------------------------------------------------------------

} // namespace

// end

