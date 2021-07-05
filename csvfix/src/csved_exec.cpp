//---------------------------------------------------------------------------
// csved_exec.cpp
//
// execute external command against each CSV input record
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_exec.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_exec.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register exec command
//---------------------------------------------------------------------------

static RegisterCommand <ExecCommand> rc1_(
	CMD_EXEC,
	"execute external command"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const EXEC_HELP = {
	"execute external command with CSV as parameters\n"
	"usage: csvfix exec [flags] [file ...]\n"
	"where flags are:\n"
	"  -c cmd\tcommand line to execute\n"
	"  -r\t\treplace CSV input with command output\n"
	"#ALL,SKIP,PASS"
};

//------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

ExecCommand ::ExecCommand( const string & name,
								const string & desc )
		: Command( name, desc, EXEC_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_CMD, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_REPLACE, false, 0 ) );
}

//---------------------------------------------------------------------------
// Execute commands against CSV input. If the -r flag is used, output result
// only else append result to input CSV.
//---------------------------------------------------------------------------

int ExecCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	mCmdLine = cmd.GetValue( FLAG_CMD, "" );
	if ( ALib::IsEmpty( mCmdLine ) ) {
		CSVTHROW( "Empty command" );
	}
	bool csv = ! cmd.HasFlag( FLAG_REPLACE );

	IOManager io( cmd );
	CSVRow row;
	ALib::Executor ex;

	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}
		string cmd = MakeCmd( row );
		std::istream & is = ex.Exec( cmd );
		if ( ! is ) {
			CSVTHROW( "Command execution error" );
		}

		string line;
		ALib::CSVLineParser clp;

		while( std::getline( is, line ) ) {
			if ( csv ) {
				CSVRow tmp( row ), cmdout;
				if ( ! Pass( tmp ) ) {
					clp.Parse( line, cmdout );
					ALib::operator+=( tmp, cmdout );
					io.WriteRow( tmp );
				}
			}
			else {
				io.Out() << line << "\n";
			}
		}
	}

	return 0;
}

//----------------------------------------------------------------------------
// Helpers to make the command string from the -c flag value and the CSV
// input data.
//----------------------------------------------------------------------------

const char PARAMC = '%';		// param intro character

string ExecCommand :: MakeCmd( const CSVRow & row ) {

	string cmd;
	unsigned int pos = 0;

	while( char c = ALib::Peek( mCmdLine, pos ) ) {
		if ( c == PARAMC ) {
			cmd += MakeParam( row, ++pos );
		}
		else {
			cmd += c;
			pos++;
		}
	}
	return cmd;
}

//----------------------------------------------------------------------------
// Called with pos set to first charcter of parameter after '%' intro char.
// We allow MAX_PARAM parameters - actual number is not significant.
//----------------------------------------------------------------------------

string ExecCommand :: MakeParam( const CSVRow & row, unsigned int  & pos ) {

	const int MAX_PARAM = 100;

	char c = ALib::Peek( mCmdLine, pos );
	if ( c == PARAMC ) {
		pos++;
		return ALib::Str( PARAMC );
	}
	else if ( std::isdigit( c ) ) {
		string p;
		while ( std::isdigit( c ) ) {
			pos++;
			p += c;
			c = ALib::Peek( mCmdLine, pos );
		}
		int n = ALib::ToInteger( p ) - 1;
		if ( n < 0 || n > MAX_PARAM ) {
			CSVTHROW( "Invalid parameter: %" << p );
		}

// different quoting rules for windows and unix
#ifdef _WIN32
		return (unsigned int) n < row.size()
					? row[n]
					: "";
#else
		return (unsigned int) n < row.size()
					? ALib::Escape( row[n], "\\'\"", "\\" )
					: "";
#endif
	}
	else {
		ATHROW( "Invalid parameter" );
	}
}

//------------------------------------------------------------------------

} // end namespace

// end

