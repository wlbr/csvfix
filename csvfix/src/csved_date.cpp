//---------------------------------------------------------------------------
// csved_date.cpp
//
// read and write dates in iso format
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "csved_date.h"
#include "csved_except.h"
#include "csved_cli.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register date commands
//---------------------------------------------------------------------------

static RegisterCommand <DateReadCommand> rc1_(
	CMD_DREAD,
	"convert dates to ISO format"
);

static RegisterCommand <DateFormatCommand> rc2_(
	CMD_DFMT,
	"perform date formatting"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const DREAD_HELP = {
	"converts date fields to ISO YYYY-MM-DD format\n"
	"usage: csvfix date_iso [flags] [files ...]\n"
	"where flags are:\n"
	"  -f fields\tspecify fields using numeric field indexes\n"
	"  -m mask\tspecifies order of date components in d/m/y form\n"
	"  -cy year\tspecifies base year for 2-digit year values\n"
	"  -mn names\tspecifies month names - default is English months\n"
	"  -bxl\t\tlist only records containing invalid dates\n"
	"  -bdx\t\tsliently exclude records containing invalid dates\n"
	"#SMQ,SEP,IBL,IFN,OFL,SKIP,PASS"
};

const char * const DFMT_HELP = {
	"format dates for output\n"
	"usage: csvfix date_format [flags] [files ...]\n"
	"where flags are:\n"
	"  -f fields\tspecify fields using numeric field indexes\n"
	"  -fmt fmt\tformat to to ue for output\n"
	"#SMQ,SEP,IBL,IFN,OFL,SKIP,PASS"
};

//---------------------------------------------------------------------------
// List of default month names - must be comma separated
//---------------------------------------------------------------------------

const char * MONTH_NAMES =  "January,February,March,April,May,June,"
							"July,August,"
							"September,October,November,December";

//---------------------------------------------------------------------------
// Year at which 2-digit wrap occurs
//---------------------------------------------------------------------------

const int BASE_YEAR = 1930;

//------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

DateReadCommand :: DateReadCommand( const string & name,
								const string & desc )
		: Command( name, desc, DREAD_HELP ), mReader( 0 ),
				mWriteAction( WriteAll ) {
	AddFlag( ALib::CommandLineFlag( FLAG_COLS, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_MASK, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_CDATE, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_MNAMES, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_BDLIST, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_BDEXCL, false, 0 ) );

}

//---------------------------------------------------------------------------
// Scrap the date reader
//---------------------------------------------------------------------------

DateReadCommand :: ~DateReadCommand() {
	delete mReader;
}

//---------------------------------------------------------------------------
// Read CSV data and convert specified fields to ISO
//---------------------------------------------------------------------------

int DateReadCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {

		if ( Skip( row ) ) {
			continue;
		}
		if( Pass( row ) ) {
			io.WriteRow( row );
			continue;
		}

		if ( ConvertDates( row ) ) {
			io.WriteRow( row );
		}
	}

	return 0;
}

//---------------------------------------------------------------------------
// Convert dates in a row. Return value indicates if row should be written
// by the io manager.
//---------------------------------------------------------------------------

bool DateReadCommand :: ConvertDates( CSVRow & row ) {

	bool havebad = false;
	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( ALib::Contains( mFields, i ) ) {
			string ds = row[i];
			ALib::Date dt;
			bool ok = mReader->Read( ds, dt );
			if ( ok ) {
				row[i] = dt.Str();
			}
			else {
				havebad = true;
				// error handling not yet
			}
		}
	}

	if ( havebad ) {
		return mWriteAction == WriteAll || mWriteAction == WriteBad;
	}
	else {
		return  mWriteAction == WriteAll || mWriteAction == WriteGood;
	}
}

//---------------------------------------------------------------------------
// Process command line flags
//---------------------------------------------------------------------------

void DateReadCommand :: ProcessFlags( ALib::CommandLine & cmd ) {

	ALib::CommaList cl( cmd.GetValue( FLAG_COLS, "" ) );
	CommaListToIndex( cl, mFields );
	string mask = cmd.GetValue( FLAG_MASK, "" );
	string cys = cmd.GetValue( FLAG_CDATE, ALib::Str( BASE_YEAR ) );

	NotBoth( cmd, FLAG_BDLIST, FLAG_BDEXCL );

	if ( cmd.HasFlag( FLAG_BDLIST )  ) {
		mWriteAction = WriteBad;
	}
	else if ( cmd.HasFlag( FLAG_BDEXCL ) ) {
		mWriteAction = WriteGood;
	}
	else {
		mWriteAction = WriteAll;
	}

	if  ( ! ALib::IsInteger( cys )) {
		CSVTHROW( "Invalid year value " << cys );

	}
	int cy = ALib::ToInteger( cys );
	string mnames = cmd.GetValue( FLAG_MNAMES, MONTH_NAMES );
	delete mReader;
	mReader = new MaskedDateReader( mask, mnames, cy );
}

//---------------------------------------------------------------------------
// Date formatting stuff
//---------------------------------------------------------------------------

const string FMT_CHARS = "dmyDMYwW";

//---------------------------------------------------------------------------
// Format dates
//---------------------------------------------------------------------------

DateFormatCommand  :: DateFormatCommand( const string & name,
											const string & desc )
	: Command( name, desc, DFMT_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FMT, true, 1 ) );
}

//---------------------------------------------------------------------------
// Read rows and fformat specified fields
//---------------------------------------------------------------------------

int DateFormatCommand  :: Execute( ALib::CommandLine & cmd ) {
	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		FormatDates( row );
		io.WriteRow( row );
	}

	return 0;
}

//---------------------------------------------------------------------------
// Format dates in row specified by fields list
//---------------------------------------------------------------------------

void DateFormatCommand :: FormatDates( CSVRow & row ) {
	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( ALib::Contains( mFields, i ) ) {
			string ds = row[i];
			row[i] = FormatDate( ds );
		}
	}
}

//---------------------------------------------------------------------------
// Formt date component with subformat string
//---------------------------------------------------------------------------

static string Format( const string & fmt, const ALib::Date & date ) {

	if ( fmt == "d" ) {
		return ALib::Str( date.Day() );
	}
	else if ( fmt == "dd" ) {
		return ALib::ZeroPad( date.Day(), 2 );
	}
	else if ( fmt == "m" ) {
		return ALib::Str( date.Month() );
	}
	else if ( fmt == "mm" ) {
		return ALib::ZeroPad( date.Month(), 2 );
	}
	else if ( fmt == "mmm" ) {
		return date.ShortMonthName();
	}
	else if ( fmt == "M" ) {
		return date.MonthName();
	}
	else if ( fmt == "y" || fmt == "yyyy" ) {
		return ALib::Str( date.Year() );
	}
	else if ( fmt == "W" ) {
		return date.DayName();
	}
	else if ( fmt == "w" ) {
		return date.ShortDayName();
	}
	else {
		CSVTHROW( "Invalid date format substring: "  << fmt );
	}
}

//---------------------------------------------------------------------------
// Format string rep of date
//---------------------------------------------------------------------------

string DateFormatCommand :: FormatDate( const string & ds ) {
	int y,m,d;
	bool ok = ALib::Date::Parse( ds, y, m, d );
	if ( ok && ALib::Date::Validate( y, m, d ) == ALib::Date::DATEOK ) {
		ALib::Date date( y, m, d );
		string out;
		for ( unsigned int i = 0; i < mFormat.size(); i++ ) {
			if ( mFormat[i].mIsFmt ) {
				out += Format( mFormat[i].mText, date );
			}
			else {
				out += mFormat[i].mText;
			}
		}
		return out;
	}
	return ds;
}

//---------------------------------------------------------------------------
// Process commend line flags
//---------------------------------------------------------------------------

void DateFormatCommand :: ProcessFlags( ALib::CommandLine & cmd ) {
	ALib::CommaList cl( cmd.GetValue( FLAG_COLS, "" ) );
	CommaListToIndex( cl, mFields );
	string fmt = cmd.GetValue( FLAG_FMT, "" );
	if ( ALib::IsEmpty( fmt ) ) {
		CSVTHROW( "Empty date format" );
	}
	BuildFormat( fmt );
}

//---------------------------------------------------------------------------
// Helper to say if this char is a formatting char
//---------------------------------------------------------------------------

static bool IsFmtChar( char c ) {
	return FMT_CHARS.find( c ) != std::string::npos;
}


//---------------------------------------------------------------------------
// Add format substring and clear string
//---------------------------------------------------------------------------

void DateFormatCommand :: AddFmt( std::string & s ) {
	if ( s != "" ) {
		mFormat.push_back( FmtEntry( s, true ) );
		s = "";
	}
}

//---------------------------------------------------------------------------
// add lieral substring and clear string
//---------------------------------------------------------------------------

void DateFormatCommand :: AddLit( std::string & s ) {
	if ( s != "" ) {
		mFormat.push_back( FmtEntry( s, false ) );
		s = "";
	}
}


//---------------------------------------------------------------------------
// Build format/literal vector from substrings
//---------------------------------------------------------------------------

void DateFormatCommand :: BuildFormat( const std::string & fmt ) {
	string fs, ls;
	unsigned int pos = 0;
	char t = 0;
	while( pos < fmt.size() ) {
		char c = fmt[ pos++ ];
		if ( c == t ) {
			fs += c;
		}
		else if ( IsFmtChar( c ) ) {
			AddFmt( fs  );
			AddLit( ls );
			t = c;
			fs += c;
		}
		else {
			AddFmt( fs );
			t = 0;
			ls += c;
		}
	}
	AddFmt( fs );
	AddLit( ls );
//	ALib::Dump( std::cout, mFormat );
}


//---------------------------------------------------------------------------
// MaskedReader stuff
//---------------------------------------------------------------------------


//------------------------------------------------------------------------
// Create reader.
//
//	mask is string in a form like 'd/m/y'
//	months is a list of month names
//  ybase is year to do 2-digit wrap at
//---------------------------------------------------------------------------

MaskedDateReader :: MaskedDateReader( const string & mask,
										const string & months,
										unsigned int ybase ) {
	SetMask( mask );
	SetYearBase( ybase );
	SetMonths( months );
}


//---------------------------------------------------------------------------
// Throw if not a valid mask character
//---------------------------------------------------------------------------

static char CheckMask( char c ) {
	if ( c != 'd' && c != 'm' && c != 'y' ) {
		CSVTHROW( "Invalid character in date mask: " << c );
	}
	return c;
}

//---------------------------------------------------------------------------
// Set mask
//---------------------------------------------------------------------------

void MaskedDateReader :: SetMask( const string & mask ) {

	if ( mask.size() != 5 ) {
		CSVTHROW( "Invalid date mask: " << mask );
	}

	mSep[0] = mSep[1] = "";

	mDMY[0] = CheckMask( mask[0] );
	mSep[0] += mask[1];
	mDMY[1] = CheckMask( mask[2] );
	mSep[1] += mask[3];
	mDMY[2] = CheckMask( mask[4] );

	if ( isalnum( mSep[0][0] ) || isalnum( mSep[1][0] ) ) {
		CSVTHROW( "Invalid separator in date mask: " << mask );
	}

	int n = 'd' + 'm' + 'y';
	if ( mDMY[0] + mDMY[1] + mDMY[2] != n ) {
		CSVTHROW( "Invalid date mask: " << mask );
	}
}

//---------------------------------------------------------------------------
// set base year for 2-digit wrap
//---------------------------------------------------------------------------

void MaskedDateReader :: SetYearBase( unsigned int ybase ) {
	if ( ybase == 0 ) {
		ybase = BASE_YEAR;
	}
	mYearBase = ybase;
}

//---------------------------------------------------------------------------
// set list of month names to use for conversion
//---------------------------------------------------------------------------

void MaskedDateReader :: SetMonths( const string & months ) {
	if ( months == "" ) {
		mMonthNames = ALib::CommaList( MONTH_NAMES );
	}
	else {
		mMonthNames = ALib::CommaList( months );
	}
	if ( mMonthNames.Size() != 12 ) {
		CSVTHROW( "Invalid month list: " << months );
	}
}

//---------------------------------------------------------------------------
// Read date from string and convert to date. Return true if siccesful
//---------------------------------------------------------------------------

bool MaskedDateReader :: Read( const string & ds, ALib::Date & date ) {

	ALib::STRPOS s1 = ds.find( mSep[0] );
	ALib::STRPOS s2 = ds.find_last_of( mSep[1] );
	if ( s1 == std::string::npos  ||
		 s2 == std::string::npos  ||
		 s1 >= s2 ) {
		return false;
	}

	string s[3];
	s[0] = ds.substr( 0, s1 );
	s[1] = ds.substr( s1 + 1, (s2 - s1) - 1 );
	s[2] = ds.substr( s2 + 1 );

	int day = -1, month = -1, year = -1;

	for ( unsigned int i = 0; i < 3; i++ ) {
		switch( mDMY[i] ) {
			case 'd':	MakeDay( s[i], day ); break;
			case 'm':	MakeMonth( s[i], month ); break;
			case 'y':	MakeYear( s[i], year ); break;
		}
	}

	if ( ALib::Date::Validate( year, month, day ) != ALib::Date::DATEOK ) {
		return false;
	}

	date = ALib::Date( year, month, day );
	return true;

}

//---------------------------------------------------------------------------
// If s contains valid day, convert it
//---------------------------------------------------------------------------

void MaskedDateReader :: MakeDay( const string & s, int & day  ) {
	if ( ALib::IsInteger( s ) ) {
		day = ALib::ToInteger( s );
	}
}

//---------------------------------------------------------------------------
// If s is integer, convert and use as month value. Otherwise see if the
// string is the start of a date name  and us ethat.
//---------------------------------------------------------------------------

void MaskedDateReader :: MakeMonth( const string & s, int & month  ) {
	if ( ALib::IsInteger( s ) ) {
		month = ALib::ToInteger( s );
	}
	else if ( s.size() >= 3 ) {
		for ( unsigned int i = 0; i < 12; i++ ) {
			string ms = mMonthNames.At(i);
			if ( ms.size() >= s.size() ) {
				ms = ms.substr( 0, s.size() );
				if ( ALib::Equal( ms, s ) ) {
					month = i + 1;
					return;
				}
			}
		}
	}
}

//---------------------------------------------------------------------------
// Convert string to year, handling 2 digit wrap
//---------------------------------------------------------------------------

void MaskedDateReader :: MakeYear( const string & s, int & year  ) {
	if ( s.size() == 2 || s.size() == 4 ) {
		if ( ALib::IsInteger( s ) ) {
			year = ALib::ToInteger( s );
			if ( s.size() == 2 ) {
				if ( year < mYearBase - 1900 ) {
					year += 2000;
				}
				else {
					year += 1900;
				}
			}
		}
	}
}

//---------------------------------------------------------------------------


} // end namespace

// end

