//---------------------------------------------------------------------------
// csved_check.cpp
//
// Check CSV records actually are CSV
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include <iostream>
#include "a_base.h"
#include "csved_cli.h"
#include "csved_check.h"
#include "csved_strings.h"
#include "csved_sparse.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register check command
//---------------------------------------------------------------------------

static RegisterCommand <CheckCommand> rc1_(
	CMD_CHECK,
	"check CSV record format conforms to CSV standard"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const CHECK_HELP = {
	"check that records in file conform to CSV standards\n"
	"usage: csvfix check [flags] [files ...]\n"
	"where flags are:\n"
	" -nl\t\tallow embedded newlines in quoted fields\n"
	" -q\t\trun quietly - no output but return error indicator\n"
	" -s sep\tuse sep as the field separator (default is ',' )\n"
	" -v\t\tprint OK message for inputs that pass validation\n"

};

//------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

CheckCommand :: CheckCommand( const string & name,
								const string & desc )
		: Command( name, desc, CHECK_HELP ),
		    mQuiet( false ), mEmbedNLOk( false ), mVerbose( false ), mSep( ',' ) {
	AddFlag( ALib::CommandLineFlag( FLAG_QUIET, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_SEP, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_NLOK, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_VERBOSE, false, 0 ) );
}

//---------------------------------------------------------------------------
// Need to read input directly from the I/O manager as we want to do
// character-by-character input in the checker.
//---------------------------------------------------------------------------

int CheckCommand :: Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );

	IOManager io( cmd );
	string line;
	int errors = 0;
	CSVRow row;

	for( unsigned int i = 0; i < io.InStreamCount(); i++ ) {
		CSVChecker chk( io.CurrentFileName(), io.In( i ), mSep, true, mEmbedNLOk );
		try {
			while( chk.NextRecord( row ) ) {
			}
			if ( mVerbose ) {
				std::cout << io.CurrentFileName() << " - OK" << std::endl;
			}
		}
		catch( const CSVED::Exception & e  ) {
			if ( mQuiet ) {
				return 1;
			}
			else {
				errors++;
				std::cout << e.what() << std::endl;
			}
		}
	}
	return errors == 0 ? 0 : 1;
}



//---------------------------------------------------------------------------
// Note that not all characters are possible CSV field separators, even
// taking the widest meaning of "CSV".
//---------------------------------------------------------------------------

void CheckCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {
	mQuiet = cmd.HasFlag( FLAG_QUIET );
	mVerbose = cmd.HasFlag( FLAG_VERBOSE );
	mEmbedNLOk = cmd.HasFlag( FLAG_NLOK );
	string sep = cmd.GetValue( FLAG_SEP, "," );
	if ( sep.size() != 1 || ! std::ispunct( sep[0] ) || std::isspace( sep[0] )
			|| sep[0] == '"'  || sep[0] == '\'' ) {
		CSVTHROW( "Invalid separator" );
	}
	mSep = sep[0];
}


} // end namespace

// end

