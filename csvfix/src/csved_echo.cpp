//---------------------------------------------------------------------------
// csved_echo.cpp
//
// echo csv input to output - mainly used for testing
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "csved_cli.h"
#include "csved_echo.h"
#include "csved_except.h"
#include "csved_ioman.h"
#include "csved_strings.h"

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register echo command
//---------------------------------------------------------------------------

static RegisterCommand <EchoCommand> rc1_(
	CMD_ECHO,
	"echo input to output"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const ECHO_HELP = {
	"echo input CSV data to output\n"
	"usage: csvfix echo [flags] [file ...]\n"
	"where flags are:\n"
	"#ALL,SKIP"
};

//---------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

EchoCommand :: EchoCommand( const string & name,
							const string & desc )
	: Command( name, desc, ECHO_HELP ) {
}

//---------------------------------------------------------------------------
// Echo simply copies input to output
//---------------------------------------------------------------------------

int EchoCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( ! Skip( row ) ) {
			io.WriteRow( row );
		}
	}

	return 0;
}

//---------------------------------------------------------------------------

}	// end namespace

// end
