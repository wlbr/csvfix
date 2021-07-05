//---------------------------------------------------------------------------
// csved_edit.cpp
//
// Edit fields for CSVfix. This was originally going to be the primary
// user interface, but it turned out not to be such a good idea, and only
// the s (substitute) command is actually implemented.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "a_regex.h"
#include "csved_cli.h"
#include "csved_edit.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register find command
//---------------------------------------------------------------------------

static RegisterCommand <EditCommand> rc1_(
	CMD_EDIT,
	"edit fields"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const EDIT_HELP = {
	"performs sed-style editing on CSV data\n"
	"usage: csvfix edit [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to apply edits to (default is all fields)\n"
	"  -e cmd\tspecify edit command - currently only s(ubstitute) implemented\n"
	"#ALL,SKIP,PASS"
};

//------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

EditCommand ::EditCommand( const string & name,
								const string & desc )
		: Command( name, desc, EDIT_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_EDIT, true, 1, true ) );
	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1, false ) );
}

//---------------------------------------------------------------------------
// Get the subcommands and then apply them to inputs
//---------------------------------------------------------------------------

int EditCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	for ( int i = 2; i < cmd.Argc(); i++ ) {
		if ( cmd.Argv( i ) == FLAG_EDIT ) {
			AddSubCmd( cmd.Argv( i + 1 ) );
			i++;
		}
	}

	ALib::CommaList cl( cmd.GetValue( FLAG_COLS, "" ) );
	CommaListToIndex( cl, mCols );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}
		if ( ! Pass( row ) ) {
			EditRow( row );
		}
		io.WriteRow( row );
	}

	return 0;
}

//---------------------------------------------------------------------------
// Edit row by allplying all sub-commands to specified columns
//---------------------------------------------------------------------------

void EditCommand :: EditRow( CSVRow & row ) {
	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( mCols.size() == 0 || ALib::Contains( mCols, i ) ) {
			EditField( row[i] );
		}
	}
}

//---------------------------------------------------------------------------
// Edit single field using subcommands. Currently only the 's' subcommand
// is supported, which works like that in vi. e.g.
//
//		s/abc/XXX/g
//
// would change all occurrences of abc into XXX
//---------------------------------------------------------------------------

const char SUB_CMD 	= 's';		// substitute command
const char IC_OPT		= 'i';		// ignore case option
const char ALL_OPT		= 'g';		// replace all (global) option

void EditCommand :: EditField( std::string & f ) {
	for ( unsigned int i = 0; i < mSubCmds.size(); i++ ) {
		EditSubCmd sc = mSubCmds[ i ];
		if ( sc.mCmd == SUB_CMD ) {
			if ( sc.mFrom == "" ) {
				CSVTHROW( "Need expression to search for" );
			}
			bool icase = sc.mOpts.find( IC_OPT ) != std::string::npos ;
			ALib::RegEx r( sc.mFrom,
							icase ? ALib::RegEx::Insensitive
							      : ALib::RegEx::Sensitive );
			if ( sc.mOpts.find( ALL_OPT ) != std::string::npos ) {
				r.ReplaceAllIn( f, sc.mTo );
			}
			else {
				r.ReplaceIn( f, sc.mTo );
			}
		}
		else {
			CSVTHROW( "Invalid edit sub-comand: " << sc.mCmd  );
		}
	}
}

//----------------------------------------------------------------------------
// Helper to read a character from a string which must not be the EOS
//----------------------------------------------------------------------------

static char MustGet( const string & s, unsigned int  i ) {
	char c = ALib::Peek( s, i );
	if ( c == 0 ) {
		CSVTHROW( "Invalid value for " << FLAG_EDIT << ": " << s );
	}
	return c;
}

//----------------------------------------------------------------------------
// Helper to read a string up to the specified separator. Escaped separators
// must be skipped over.
//----------------------------------------------------------------------------

static string ReadField( const string & s, unsigned int & i, char sep ) {
	string f = "";
	bool escaped = false;
	while(1) {
		char c = MustGet( s, i++ );
		if ( escaped ) {
			escaped = false;
			f += c;
		}
		else if ( c == '\\' ) {
			escaped = true;
			f += c;
		}
		else if ( c != sep ) {
			f += c;
		}
		else if ( c == sep ) {
			break;
		}
	}
	return f;
}


//----------------------------------------------------------------------------
// Parse a subs command of the form cmd/pattern/subs/opt, where cmd is always
// a single character, / may be any and character except the backslash, and
// opt is optional.
//----------------------------------------------------------------------------

static void ParseSub( const string & s, char & cmd, vector <string> & fields ) {

	unsigned int i = 0;
	cmd = MustGet( s, i++ );
	if ( cmd != SUB_CMD ) {
		CSVTHROW( "Invalid value for " << FLAG_EDIT << ": " << s );
	}
	char sep = MustGet( s, i++ );
	if (  sep == '\\' ) {
		CSVTHROW( "Invalid value for " << FLAG_EDIT << ": " << s );
	}

	string f = ReadField( s, i, sep );
	fields.push_back( f );
	f = ReadField( s, i, sep );
	fields.push_back( f );
	f = s.substr( i );
	fields.push_back( f );
}

//---------------------------------------------------------------------------
// Add sub command with some error checking.
// No longer uses Split() to parse command as that did  not deal correctly
// with escaped separators and empty command fields.
//---------------------------------------------------------------------------

void EditCommand :: AddSubCmd( const string & ev ) {
	if ( ALib::IsEmpty( ev ) ) {
		CSVTHROW( "Empty value for " << FLAG_EDIT );
	}

	if ( ! isalpha( ev[0] ) ) {			// for now
		CSVTHROW( "Edit sub command missing from " << ev );
	}

	char cmd;
	vector <string> tmp;
	ParseSub( ev, cmd, tmp );
//	for ( unsigned int i = 0; i < tmp.size(); i++ ) {
//		std::cout << "[" << tmp[i] << "]" << std::endl;
//	}

	mSubCmds.push_back ( EditSubCmd( cmd, tmp[0], tmp[1], tmp[2] ) );
}

//------------------------------------------------------------------------

} // namespace

// end

