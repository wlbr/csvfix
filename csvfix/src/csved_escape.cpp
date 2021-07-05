//---------------------------------------------------------------------------
// csved_escape.cpp
//
// Do special character quoting, by default using backslash character.
// Now called 'escape', not 'quote',
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_escape.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register escape command
//---------------------------------------------------------------------------

static RegisterCommand <EscCommand> rc1_(
	CMD_ESC,
	"escape special characters"
);

//----------------------------------------------------------------------------
// String to use for escaping
//----------------------------------------------------------------------------

const char * const DEF_ESC = "\\";

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const ESC_HELP = {
	"performs escaping of special characters\n"
	"usage: csvfix escape [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to apply escaping to (default is all fields)\n"
	"  -s chars\tlist of characters to be escaped\n"
	"  -e esc\tstring to use for escaping (default is backslash)\n"
	"  -sql\t\tperform SQL single quote escaping\n"
	"  -noc\t\tturns off CSV interenal escaping\n"
	"#ALL,SKIP,PASS"
};

//---------------------------------------------------------------------------
// standard command constructor
//---------------------------------------------------------------------------

EscCommand :: EscCommand( const string & name,
								const string & desc )
		: Command( name, desc, ESC_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_SQLQ, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_CHARS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ESC, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ESCOFF, false, 0 ) );

}

//---------------------------------------------------------------------------
// Read inputs and perform escaping on specified cols or all if none were
// specified with fields flag.
//
// The -sql flag sets the correct values for sql quoting. The value of the
// -esc flag is automatically added to special characters if it is a single
// character, otherwise the user has to deal with it explicitly.
//---------------------------------------------------------------------------

int EscCommand ::	Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	mSqlMode = cmd.HasFlag( FLAG_SQLQ );
	if ( mSqlMode ) {
		if ( cmd.HasFlag( FLAG_CHARS ) || cmd.HasFlag( FLAG_ESC ) ) {
			CSVTHROW( "Cannot specify " << FLAG_SQLQ << " with "
						<< FLAG_CHARS << " or " << FLAG_ESC );
		}
		mEsc = "'";
		mSpecial = "'";
	}
	else {
		mSpecial = cmd.GetValue( FLAG_CHARS );
		if ( mSpecial == "" ) {
			CSVTHROW( FLAG_CHARS << " needs characters to escape" );
		}
		mEsc = cmd.GetValue( FLAG_ESC, DEF_ESC );
		if ( mEsc.size() == 1 ) {
			mSpecial += mEsc;
		}
	}

	GetColumns( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}
		if( ! Pass( row ) ) {
			EscapeRow( row );
		}
		io.WriteRow( row, cmd.HasFlag( FLAG_ESCOFF) );
	}

	return 0;
}

//---------------------------------------------------------------------------
// Get columns to apply escaping to. If none specified, apply to all.
//---------------------------------------------------------------------------

void EscCommand :: GetColumns( const ALib::CommandLine & cmd ) {
	mCols.clear();
	string cs = cmd.GetValue( FLAG_COLS );
	if ( ! ALib::IsEmpty( cs ) ) {
		ALib::CommaList cl( cs );
		CommaListToIndex( cl, mCols );
	}
}

//---------------------------------------------------------------------------
// Escape values in row, replacing existing values.
//---------------------------------------------------------------------------

void EscCommand :: EscapeRow( CSVRow & row ) {

	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( mCols.size() == 0 || ALib::Contains( mCols, i  ) ) {
			EscapeValue( row, i );
		}
	}
}

//---------------------------------------------------------------------------
// Escape specified row value
//---------------------------------------------------------------------------

void EscCommand :: EscapeValue( CSVRow & row, unsigned int i ) {
	string & val = row[i];
	if ( mSqlMode ) {
		val = ALib::SQLQuote( val );
	}
	else if ( val.find_first_of( mSpecial ) != std::string::npos  ) {
		string s;
		for ( unsigned int i = 0; i < val.size(); i++ ) {
			char c = val[ i ];
			if ( mSpecial.find( c ) != std::string::npos  ) {
				s += mEsc;
			}
			s += c;
		}
		val = s;
	}
}

//------------------------------------------------------------------------

} // end namespace

// end

