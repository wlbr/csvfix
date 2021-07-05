//---------------------------------------------------------------------------
// csved_number.cpp
//
// convert fields possibly containing strange thousands/decimal point
// separators to numeric values
//
// Copyright (C) 2013 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_number.h"
#include "csved_strings.h"

#include <string>
using std::string;

namespace CSVED {


//---------------------------------------------------------------------------
// Register number command
//---------------------------------------------------------------------------

static RegisterCommand <NumberCommand> rc1_(
	CMD_NUMBER,
	"convert formatted numeric fields to ordinary numeric"
);


//----------------------------------------------------------------------------
// Help
//----------------------------------------------------------------------------

const char * const NUMBER_HELP = {
	"convert formatted numeric fields to ordinary numeric\n"
	"usage: csvfix number  [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to convert - default is all\n"
	"  -fmt fmt\tformat of input fields - one of EU or EN (default)\n"
	"  -es str\treplace fields that cannot be read with this string\n"
	"  -ec \t\tconversion failure is an error\n"
	"#ALL"
};

//----------------------------------------------------------------------------
// The number command
//----------------------------------------------------------------------------

NumberCommand :: NumberCommand( const string & name, const string & desc )
				: Command( name, desc, NUMBER_HELP ),
					mErrExit( false ), mHasErrStr( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FMT, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ERRSTR, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ERRCODE, false, 0 ) );
}

//----------------------------------------------------------------------------
// Two supported formats:
//
//    1,234.00	---> EN
//    1.234,00	---> EU
//----------------------------------------------------------------------------

const string EN_FMT	= "EN";
const string EU_FMT	= "EU";

//----------------------------------------------------------------------------
// Convert input rows, with Skip/Pass filtering.
//----------------------------------------------------------------------------

int NumberCommand :: Execute( ALib::CommandLine & cmd )  {

	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {

		if ( Skip( row ) ) {
			continue;
		}
		if( ! Pass( row ) ) {
			Convert( row );
		}

		io.WriteRow( row );
	}

	return 0;
}


//----------------------------------------------------------------------------
// Convert specified value using language-specific thousands separator and
// decimal point.
//----------------------------------------------------------------------------

string NumberCommand :: ConvertField( const string & field, char ts, char dp ) {

	string s;
	bool havedp = false;
	for( unsigned int i = 0; i < field.size(); i++ ) {
		char c = field[i];
		if ( c == dp ) {
			havedp = true;
			if ( dp != '.' ) {
				c = '.';
			}
		}
		else if ( c == ts && ! havedp ) {
			continue;
		}
		s += c;
	}

	if ( ! ALib::IsNumber( s ) ) {
		if ( mErrExit ) {
			CSVTHROW( "Invalid number: " << field );
		}
		else if ( mHasErrStr ) {
			return mErrStr;
		}
		else {
			return field;
		}
	}

	return s;
}


//----------------------------------------------------------------------------
// Convert specified fields
//----------------------------------------------------------------------------

void NumberCommand :: Convert( CSVRow & row ) {
	for( unsigned int i = 0; i < row.size(); i++ ) {
		if ( mFields.size() == 0 || ALib::Contains( mFields, i ) ) {
			char ts = mFormat == EN_FMT ? ',' : '.';
			char dp = mFormat == EN_FMT ? '.' : ',';
			row[i] = ConvertField( row[i], ts, dp );
		}
	}
}

//----------------------------------------------------------------------------
// Get command line options and report any problems  with them.
//----------------------------------------------------------------------------

void NumberCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	string cs = cmd.GetValue( FLAG_COLS );
	if ( ! ALib::IsEmpty( cs ) ) {
		ALib::CommaList cl( cs );
		CommaListToIndex( cl, mFields );
	}
	mFormat = cmd.GetValue( FLAG_FMT, EN_FMT);
	if ( mFormat != EN_FMT && mFormat != EU_FMT ) {
		CSVTHROW( FLAG_FMT << " must be " << EN_FMT << " or " << EU_FMT );
	}
	mErrExit = cmd.HasFlag( FLAG_ERRCODE );
	mErrStr = cmd.GetValue( FLAG_ERRSTR );
	mHasErrStr = cmd.HasFlag( FLAG_ERRSTR );
	if ( mErrExit && mHasErrStr ) {
		CSVTHROW( "Cannot specify both " << FLAG_ERRCODE << " and " << FLAG_ERRSTR );
	}
}


//----------------------------------------------------------------------------

} // namespace





