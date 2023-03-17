//---------------------------------------------------------------------------
// csved_erase.cpp
//
// Erase fields from CSV records using regualr expressions
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_erase.h"
#include "csved_strings.h"
#include "csved_evalvars.h"

using std::string;
using std::vector;
using namespace std;

namespace CSVED {

//---------------------------------------------------------------------------
// Register exclude command
//---------------------------------------------------------------------------

static RegisterCommand <EraseCommand> rc1_(
	CMD_ERASE,
	"erase fields using regular expressions"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const ERASE_HELP = {
	"erase fields using regular expressions"
	"usage: csvfix erase  [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tlist of fields to check for exclusion (default is all)\n"
	"  -r regexp\terase fields that match this regular expression\n"
    "  -n regex\terase fields which do not match this regular expression\n"
	"  -k\t\tif all fields are erased, retain empty row (default is to delete it)\n"
	"#ALL,SKIP,PASS"
};

//----------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

EraseCommand ::EraseCommand( const string & name,
								const string & desc )
		: Command( name, desc, ERASE_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_REGEX, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_NOTRE, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_KEEP, false, 0 ) );

}

//---------------------------------------------------------------------------
// Get user-specified options and then read input, removing erased cols.
//---------------------------------------------------------------------------

int EraseCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row, newrow;

	while( io.ReadCSV( row ) ) {
		if ( Skip( io, row ) ) {
			continue;
		}
		if ( ! Pass( io, row ) ) {
            newrow = EraseFields( row );
		}
        if ( newrow.size() != 0 || mKeep ) {
            io.WriteRow( newrow );
        }
	}

	return 0;
}

//---------------------------------------------------------------------------
// Should this field be erased?
//---------------------------------------------------------------------------

bool EraseCommand :: EraseField( const std::string & field ) const{
    for ( unsigned int i = 0; i < mExprs.size(); i++ ) {
        ALib::RegEx::Pos pos = mExprs[i].mRegex.FindIn( field );
        if ( (pos.Found() && mExprs[i].mMatch)
              || (! pos.Found() && ! mExprs[i].mMatch ) ) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------
// Erase fields that match/do not match regexes, returning new row without
// the erased fields.
//---------------------------------------------------------------------------

CSVRow EraseCommand :: EraseFields( const CSVRow & row ) const {
    CSVRow newrow;
    for( unsigned int i = 0; i < row.size(); i++ ) {
        if ( mFields.size() == 0 || ALib::Contains( mFields, i ) ) {
            if ( ! EraseField( row[i] ) ) {
                newrow.push_back( row[i] );
            }
        }
        else {
            newrow.push_back( row[i] );
        }
    }
    return newrow;
}

//---------------------------------------------------------------------------
// Handle all user options with error checking.
//---------------------------------------------------------------------------

void EraseCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

    if ( ! cmd.HasFlag( FLAG_REGEX ) && ! cmd.HasFlag( FLAG_NOTRE ) ) {
        CSVTHROW( "Need at least one of " << FLAG_REGEX << " or " << FLAG_NOTRE );
    }

    for(  int i = 1; i < cmd.Argc(); i++ ) {
        if ( cmd.Argv(i) == FLAG_REGEX || cmd.Argv(i) == FLAG_NOTRE ) {
            mExprs.push_back( RegexAction( ALib::RegEx( cmd.Argv(i+1)), cmd.Argv(i) == FLAG_REGEX ));
            i++;
        }
    }
    mKeep = cmd.HasFlag( FLAG_KEEP );
    CommaListToIndex( ALib::CommaList( cmd.GetValue( FLAG_COLS) ), mFields );

}



} // end namespace

// end

