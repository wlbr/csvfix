//---------------------------------------------------------------------------
// csved_money.cpp
//
// money formatting for csvfix
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "csved_cli.h"
#include "csved_money.h"
#include "csved_except.h"
#include "csved_ioman.h"
#include "csved_strings.h"
#include "a_collect.h"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register money  command
//---------------------------------------------------------------------------

static RegisterCommand <MoneyCommand> rc1_(
	CMD_MONEY,
	"format fields as money/currency values"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const MONEY_HELP = {
	"format fields as money/currency values\n"
	"usage: csvfix money [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to apply format to - default is all fields\n"
	"  -dp chr\tuse character chr as decimal point symbol - default is full-stop\n"
	"  -ts chr\tuse character chr as thousands separator - default is comma\n"
	"  -cs sym\tuse string sym as currency symbol - default is none\n"
	"  -ms minus\tuse string minus as prefix for negative values - default is \"-\"\n"
	"  -ps plus\tuse string plus as prefix for positive values - default is none\n"
	"  -cn\t\ttreat the amount being formatted as if it were cents, not dollars\n"
	"  -r\t\treplace fields with new format - default is to append fields to output\n"
	"  -w width\tspecify width of output, which will be right-aligned\n"
	"#ALL,SKIP,PASS"
};

//---------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

MoneyCommand :: MoneyCommand( const string & name,
							const string & desc )
	: Command( name, desc, MONEY_HELP ),
			mDecimalPoint( '.' ), mThouSep( ',' ), mSymbol( "" ),
			mWidth( 0 ), mCents( false ) {
	AddFlag( ALib::CommandLineFlag( FLAG_DPOINT, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_KSEP, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_CURSYM, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_REPLACE, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_PLUS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_MINUS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_WIDTH, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_CENTS, false, 0 ) );
}

//---------------------------------------------------------------------------
// Execute command, converting numbers to currency representation and
// leaving non-numeric values untouched.
//---------------------------------------------------------------------------

int MoneyCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );
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

		CSVRow out( row );
		for ( unsigned int i = 0; i < row.size(); i++ ) {
			if ( mFields.empty() || ALib::Contains( mFields, i ) ) {
				if ( mReplace ) {
					out[i] = FormatValue( row[i] );
				}
				else {
					out.push_back( FormatValue( row[i] ) );
				}
			}
		}
		io.WriteRow( out );
	}

	return 0;
}

//---------------------------------------------------------------------------
// Get command line options
//----------------------------------------------------------------------------

void MoneyCommand :: ProcessFlags( ALib::CommandLine & cmd ) {

	mCents = cmd.HasFlag( FLAG_CENTS );
	mSymbol = cmd.GetValue( FLAG_CURSYM, "" );
	mPlus = cmd.GetValue( FLAG_PLUS, "" );
	mMinus = cmd.GetValue( FLAG_MINUS, "-" );
	string fields = cmd.GetValue( FLAG_COLS, "" );
	string dp = cmd.GetValue( FLAG_DPOINT, "." );
	if ( dp.size() != 1 ) {
		CSVTHROW( "Invalid decimal point value" );
	}
	string ts = cmd.GetValue( FLAG_KSEP, "," );
	if ( dp.size() > 1 ) {
		CSVTHROW( "Invalid thousand separator value" );
	}
	mDecimalPoint = dp[0];
	mThouSep = ts == "" ? 0 : ts[0];

	if ( ! fields.empty() ) {
		CommaListToIndex( ALib::CommaList( fields ), mFields );
	}
	mReplace = cmd.HasFlag( FLAG_REPLACE );

	string ws = cmd.GetValue( FLAG_WIDTH, "0" );
	if ( ALib::IsInteger( ws ) ) {
		mWidth = ALib::ToInteger( ws );
	}
	else {
		CSVTHROW( "Width specified by " << FLAG_WIDTH << " must be integer" );
	}
	if ( mWidth < 0 || mWidth > 50 ) {		// arbitrary max width
		CSVTHROW( "Invalid width specified by " << FLAG_WIDTH << ": "  << ws );
	}
}

//----------------------------------------------------------------------------
// Turn string (if it is a number) into currency format. Non-numerics are not
// formatted. Numerics are formatted as positive numbers, and then a custom
// sign value is prepended.
//----------------------------------------------------------------------------

string MoneyCommand :: FormatValue( const string & v ) const {

	// must be a number
	if ( ! ALib::IsNumber( v ) ) {
		return v;
	}

	// do all formatting with positive numbers and adjust sign at end
	string sign = "";
	double m = ALib::ToReal( v );

	// is value to be treated as cents i.e. 123 rather than 1.23?
	if ( mCents ) {
		m /= 100;
	}

	if ( m < 0.0 ) {
		sign = "-";
		m = std::fabs( m );
	}

	// get the value into xx...xx.yy format
	std::ostringstream os;
	os << std::fixed << std::setprecision(2) << m;
	string fs  = os.str();

	// replace thousands sep and decimal point
	string cents = fs.substr( fs.size() - 2, 2 );
	string dollars = fs.substr( 0, fs.size() - 3 );
	string dsep = "";
	int dcount = 0;
	for ( int i = (int) dollars.size() - 1; i >= 0; i-- ) {
		dsep += dollars[i];
		if ( ++dcount == 3 && i != 0 ) {
			if ( mThouSep ) {	// possible to have empty thousands sep
				dsep += mThouSep;
			}
			dcount = 0;
		}
	}
	std::reverse( dsep.begin(), dsep.end() );

	// right align the numeric money element
	std::ostringstream money;
	if ( mWidth ) {
		money << std::setw( mWidth );
	}
	money <<  dsep << mDecimalPoint << cents;
	string smoney = money.str();

	// add in the sign and currency symbols
	os.str("");
	os << ( sign == "-" ? mMinus : mPlus ) << mSymbol << smoney;
	return os.str();
}

//----------------------------------------------------------------------------

}	// end namespace

// end
