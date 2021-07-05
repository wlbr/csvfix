//---------------------------------------------------------------------------
// csved_fileinfo.cpp
//
// add file name and line number info to csv output
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_file.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_fileinfo.h"
#include "csved_strings.h"

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register command
//---------------------------------------------------------------------------

static RegisterCommand <FileInfoCommand> rc1_(
	CMD_FINFO,
	"add file information to output"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const INFO_HELP = {
	"add file name and line number to output\n"
	"usage: csvfix file_info  [flags] [file ...]\n"
	"where flags are:\n"
	"  -b\t\tremove path from filename\n"
	"  -tc\t\toutput file name and line number as two separate CSV fields\n"
	"#ALL,SKIP,PASS"
};

//---------------------------------------------------------------------------
// Standard ctor
//---------------------------------------------------------------------------

FileInfoCommand :: FileInfoCommand( const string & name,
								const string & desc )
		: Command( name, desc, INFO_HELP  ),
			mBasename( false ), mTwoCols( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_TWOC, false, false ) );
	AddFlag( ALib::CommandLineFlag( FLAG_BASEN, false, false ) );
}

//---------------------------------------------------------------------------
// Read input and prepend file and line information
//---------------------------------------------------------------------------

int FileInfoCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	mTwoCols = cmd.HasFlag( FLAG_TWOC );
	mBasename = cmd.HasFlag( FLAG_BASEN );

	IOManager io( cmd );

	CSVRow row;
	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}
		if ( Pass( row ) ) {
			io.WriteRow( row );
			continue;
		}

		string fname = mBasename
						? ALib::Filename( io.CurrentFileName() ).Base()
						: io.CurrentFileName();

		CSVRow outrow;
		if ( mTwoCols ) {
			outrow.push_back( fname );
			outrow.push_back( ALib::Str( io.CurrentLine() ));
		}
		else {
			outrow.push_back( fname + " ("
				+ ALib::Str( io.CurrentLine() ) + ")"
			);
		}
		ALib::operator+=( outrow, row );
		io.WriteRow( outrow );
	}

	return 0;
}


//------------------------------------------------------------------------

} // end namespace

// end

