//---------------------------------------------------------------------------
// csved_trim.cpp
//
// trim leading/trailing spaces
// now also can trim field down to fixed width
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_collect.h"
#include "csved_cli.h"
#include "csved_trim.h"
#include "csved_strings.h"

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register template command
//---------------------------------------------------------------------------

static RegisterCommand <TrimCommand> rc1_(
	CMD_TRIM,
	"trim leading/trailing spaces"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const TRIM_HELP = {
	"trim leading & trailing spaces, or truncate to width\n"
	"usage: csvfix trim  [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to trim (default is all)\n"
	"  -l\t\ttrim leading whitespace\n"
	"  -t\t\ttrim trailing whitespace\n"
	"  -w widths\tspecifies widths to truncate fields to\n"
	"#ALL,SKIP,PASS"
};

//---------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

TrimCommand ::	TrimCommand( const string & name,
									 const string & desc )
		: Command( name, desc, TRIM_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_TRLEAD, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_TRTRAIL, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_WIDTH, false, 1 ) );
}

//----------------------------------------------------------------------------
// Get widths to trubcate fields to. Note a negative field width means that
// the field should not be truncated.
//----------------------------------------------------------------------------

void TrimCommand ::	GetWidths( const std::string & ws ) {
	ALib::CommaList wl( ws );
	for ( unsigned int i = 0; i < wl.Size(); i++ ) {
		if ( ! ALib::IsInteger( wl.At(i) ) ) {
			CSVTHROW( "Invalid width " << wl.At(i) );
		}
		int n = ALib::ToInteger( wl.At(i) );
		mWidths.push_back( n );
	}
}

//---------------------------------------------------------------------------
// Read input rows and apply trim functions depending on flags
//---------------------------------------------------------------------------

int TrimCommand ::	Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	if ( cmd.HasFlag( FLAG_TRLEAD ) || cmd.HasFlag( FLAG_TRTRAIL ) ) {
		mTrimLead = cmd.HasFlag( FLAG_TRLEAD );
		mTrimTrail = cmd.HasFlag( FLAG_TRTRAIL );
	}
	else {
		mTrimLead = mTrimTrail = true;
	}

	if ( cmd.HasFlag( FLAG_WIDTH ) ) {
		GetWidths( cmd.GetValue( FLAG_WIDTH ) );
	}

	ALib::CommaList cl( cmd.GetValue( FLAG_COLS, "" ) );
	CommaListToIndex( cl, mFields );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}

		if ( ! Pass( row ) ) {
			Trim( row );
		}
		io.WriteRow( row );
	}

	return 0;
}

//----------------------------------------------------------------------------
// Chop field down to N leftmost characters contained in mWidths member.
//----------------------------------------------------------------------------

void TrimCommand :: Chop( CSVRow & row, unsigned int  i ) {
	if ( mFields.size() == 0 ) {
		if ( i < mWidths.size()  && i < row.size() ) {
			if ( mWidths.at(i) >= 0 ) {
				row.at(i) = row.at(i).substr( 0, mWidths.at(i) );
			}
		}
	}
	else {
		int idx =  ALib::IndexOf( mFields, i );
		if ( idx >= 0 ) {
			if ( idx < (int) mWidths.size()  && i < row.size() ) {
				if ( mWidths.at(idx) >= 0 ) {
					row.at(i) = row.at(i).substr( 0, mWidths.at(idx) );
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
// Trim fields of row. Always perform whitespace removal and only perform
// width reductions if widths specified.
//---------------------------------------------------------------------------

void TrimCommand :: Trim( CSVRow & row ) {
	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( mFields.size() == 0 || ALib::Contains( mFields, i ) ) {
			if ( mTrimLead && mTrimTrail ) {
				row[i] = ALib::Trim( row[i] );
			}
			else if ( mTrimLead ) {
				row[i] = ALib::LTrim( row[i] );
			}
			else {
				row[i] = ALib::RTrim( row[i] );
			}
		}

		if (  mWidths.size() ) {
			if ( mFields.size() == 0 || ALib::Contains( mFields, i ) ) {
				Chop( row, i );
			}
		}
	}
}


//------------------------------------------------------------------------

} // end namespace

// end

