//---------------------------------------------------------------------------
// csved_rmnewline.cpp
//
// remove embedded newlines from CSV input
//
// Copyright (C) 2011 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_strings.h"
#include "csved_rmnewline.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register command
//---------------------------------------------------------------------------

static RegisterCommand <RemoveNewlineCommand> rc1_(
	CMD_RMNEW,
	"remove embedded newlines"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const RMNEW_HELP = {
	"remove embedded newlines in CSV data\n"
	"usage: csvfix rmnew [flags] [files ...]\n"
	"where flags are:\n"
	"  -s sep\tseparator text to replace newline\n"
	"  -x\t\texclude data after first newline in field\n"
	"  -f fields\tfields to apply command to (default is all)\n"
	"#SMQ,SEP,IBL,IFN,OFL,SKIP,PASS"
};



//----------------------------------------------------------------------------
// Standard command ctor
//----------------------------------------------------------------------------

RemoveNewlineCommand :: RemoveNewlineCommand( const string & name,
												const string & desc )
		: Command( name, desc, RMNEW_HELP ), mSep( "" ) {

	AddFlag( ALib::CommandLineFlag( FLAG_STR, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_EXCLNL, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
}

//---------------------------------------------------------------------------
// Get options and then process CSV input
//---------------------------------------------------------------------------

int RemoveNewlineCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}
		if ( ! Pass( row ) ) {
			RemoveNewlines( row );
		}
		io.WriteRow( row );
	}

	return 0;
}

//----------------------------------------------------------------------------
// Perform all newline removal ops on row
//----------------------------------------------------------------------------


void RemoveNewlineCommand :: RemoveNewlines( CSVRow & row ) {

	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( (mFields.size() == 0 || ALib::Contains( mFields, i ) )
				&& row[i].find_first_of( "\n" ) != std::string::npos ) {
			string s;
			for ( unsigned int j = 0; j < row[i].size(); j++ ) {
				if ( row[i][j] == '\n' ) {
					if ( mExcludeAfter ) {
						break;
					}
					else {
						s += mSep;
					}
				}
				else {
					s += row[i][j];
				}
			}
			row[i] = s;
		}
	}
}

//---------------------------------------------------------------------------
// Helper to expand any escaped characters in separator string - currently
// we only support \t for tabs.
//---------------------------------------------------------------------------

void RemoveNewlineCommand :: ExpandSep( void )  {
	string s;
	for ( unsigned int i = 0; i < mSep.size(); i++ ) {
		char c = mSep[i];
		if ( c == '\\' ) {
			if ( i == mSep.size() - 1) {
				CSVTHROW( "Invalid escape at end of string" );
			}
			c = mSep[++i];
			if ( c == 't' ) {
				s += '\t';
			}
			else {
				s += c;
			}
		}
		else {
			s += c;
		}

	}
	mSep = s;
}

//---------------------------------------------------------------------------
// Handle all user options with error checking
//---------------------------------------------------------------------------

void RemoveNewlineCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	string scols = cmd.GetValue( FLAG_COLS, "" );
	ALib::CommaList cl( cmd.GetValue( FLAG_COLS, "" ) );
	CommaListToIndex( cl, mFields);
	if ( cmd.HasFlag( FLAG_EXCLNL ) && cmd.HasFlag( FLAG_STR ) ) {
		CSVTHROW( "Flags " << FLAG_EXCLNL << " and " << FLAG_STR
					<< " are mutually exclusive" );
 	}
	mSep = cmd.GetValue( FLAG_STR, "" );
	ExpandSep();
	mExcludeAfter = cmd.HasFlag( FLAG_EXCLNL );
}

//---------------------------------------------------------------------------

} // end namespace

// end

