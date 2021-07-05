//---------------------------------------------------------------------------
// a_date.cpp
//
// date handling for alib
//
// Note:
//
//	- all d,m,y values are 1-based
//
// This software is licensed under the Apache 2.0 license. You should find
// a copy of the license in the file LICENSE.TXT. If this file is missing,
// a copy can be obtained from www.apache.org.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_date.h"
#include "a_str.h"
#include "a_except.h"
#include <sstream>
#include <iomanip>
#include <time.h>

using std::string;
using std::vector;

namespace ALib {

//---------------------------------------------------------------------------
// Min & max years we deal with
//---------------------------------------------------------------------------

const unsigned int MIN_YEAR = 1900;
const unsigned int MAX_YEAR = 3000;

//---------------------------------------------------------------------------
// Days of month - Feb is handled in code.
//---------------------------------------------------------------------------

const unsigned int MDAYS[12] = { 31, 28, 31, 30, 31,
								30, 31, 31, 30, 31, 30, 31 };

//---------------------------------------------------------------------------
// Helper to check date or throw on error
//---------------------------------------------------------------------------

static void CheckDate( int y, int m, int d ) {
	if ( Date::Validate( y, m, d ) != Date::DATEOK ) {
		ATHROW( "Invalid date" );
	}
}

//---------------------------------------------------------------------------
// Helper to convert Convert to julian date. The code is adapted from an
// article in the MSDN knowledge base. Returns -1 on error
//---------------------------------------------------------------------------

static int ToJulian( int year, int month, int day ) {
	CheckDate( year, month, day );
	double y = year +(month - 2.85) / 12;
	int rv  = int(int(int(367 * y) - int(y) - 0.75
				* int(y) + day)- 0.75 * int(y / 100)) + 1721115;
	return rv;
}

//---------------------------------------------------------------------------
// Helper to convert from julian back to day, month year
//---------------------------------------------------------------------------

static void FromJulian( int jn, int & year, int & month, int & day ) {
	int n0 = jn - 1721119;
	int c = int((n0 - 0.2) / 36524.25);
	int n1 = n0 + c - int(c / 4);
	int y = int((n1 - 0.2) / 365.25);
	int n2 = n1 - int( 365.25 * y);
	int m = int((n2 - 0.5) / 30.6);
	day = int(n2 - 30.6 * m + 0.5);
	if ( m > 9  ) {
		year = y + 1;
		month = m - 9;
	}
	else {
  		year = y;
	  	month = m + 3;
	}
}

//---------------------------------------------------------------------------
// Calculate day of week, starting with 1 -> Sunday, using Zeller's method.
//---------------------------------------------------------------------------

static int Zeller( int year, int month, int day ) {
	month -= 2;
	if ( month <= 0 ) {
		month += 12;
		year -= 1;
	}
	return ((day + (13 * month - 1) / 5
			+ 5 * (year % 100) / 4
			+ 7 * year / 400) % 7) + 1;
}


//------------------------------------------------------------------------
// Initialise date from a string which must either be empty or contain
// an ISO date in yyyy-mm-dd format.
//---------------------------------------------------------------------------

Date :: Date( const std::string & s )  {
	if ( s == "" ) {
		mDays = Today().mDays;
	}
	else {
		int d,m,y;
		if ( ! Parse( s, y, m, d ) ) {
			ATHROW( "Invalid date: " << s ) ;
		}
		mDays = Date( y,m,d).mDays;
	}
}

//---------------------------------------------------------------------------

Date ::  Date( int y, int m, int d ) {
	mDays = ToJulian( y, m, d );
}

//---------------------------------------------------------------------------
// Static function to parse ISO string to components
//---------------------------------------------------------------------------

bool Date :: Parse( const string & s, int & y, int & m, int & d ) {

	vector <string> tmp;
	if ( Split( s, '-', tmp ) != 3
			|| ! IsInteger( tmp[0] )
			|| ! IsInteger( tmp[1] )
			|| ! IsInteger( tmp[2] ) ) {
			return false;
	}

	bool ok = Date::Validate( y = ToInteger( tmp[0] ),
								m = ToInteger( tmp[1] ),
								d = ToInteger( tmp[2] ) ) == DATEOK;
	return ok;
}


//---------------------------------------------------------------------------
// Today's date
//---------------------------------------------------------------------------

Date Date :: Today() {
	time_t t = time(0);
	struct tm * now = localtime( & t );
	return Date( now->tm_year + 1900, now->tm_mon + 1, now->tm_mday  );
}

//---------------------------------------------------------------------------
// Static & non-staatic versions of leap year test
//---------------------------------------------------------------------------

bool Date :: IsLeapYear( int y ) {
	if ( y < int(MIN_YEAR) || y > int(MAX_YEAR) ) {
		ATHROW( "Invalid year: " << y );
	}
	return (y % 4 == 0) && ( y % 100 != 0 || y % 400 == 0 );
}

bool Date :: IsLeapYear() const {
	return IsLeapYear( Year() );
}

//---------------------------------------------------------------------------
// Date component access
//---------------------------------------------------------------------------

int Date :: Day() const {
	int d, m, y;
	FromJulian( mDays, y, m, d );
	return d;
}

int Date :: Month() const {
	int d, m, y;
	FromJulian( mDays, y, m, d );
	return m;
}

int Date :: Year() const {
	int d, m, y;
	FromJulian( mDays, y, m, d );
	return y;
}

int Date :: DayOfYear() const {
	int year = Year();
	Date ystart( year, 1, 1 );
	int day = Diff( *this, ystart );
	return day + 1;
}

int Date :: DayOfWeek() const {
	int d = Zeller( Year(), Month(), Day() );
	return d;
}

//---------------------------------------------------------------------------
// Month & Day names in English
//---------------------------------------------------------------------------


string Date :: MonthName() const {
	static const char * months[] = {
		"January", "February", "March", "April",
		"May", "June", "July", "August",
		"September", "October", "November", "December"
	};

	int m = Month();
	if ( m < 1 || m > 12 ) {
		ATHROW( "Invalid month index" );
	}
	return months[ m - 1 ];
}

string Date :: ShortMonthName() const {
	return MonthName().substr( 0, 3 );
}

string Date :: DayName() const {
	static const char * days[] = {
		"Sunday", "Monday", "Tuesday", "Wedneday",
		"Thursday", "Friday", "Saturday"
	};
	int d = DayOfWeek();
	if ( d < 1 || d > 7 ) {
		ATHROW( "Invalid day index" );
	}
	return days [ d - 1 ];
}

string Date :: ShortDayName() const {
	return DayName().substr( 0, 3 );
}


//---------------------------------------------------------------------------
// Validate date returning error flag or DATEOK if valid
//---------------------------------------------------------------------------

Date::Error Date :: Validate( unsigned int y,
								unsigned int m,
								unsigned int d ) {
	if ( y < MIN_YEAR || y > MAX_YEAR ) {
		return BADYEAR;
	}
	if ( d < 1  || d > 31 ) {
		return BADDAY;
	}
	if ( m < 1 || m > 12 ) {
		return BADMONTH;
	}
	if ( m != 2 && (d < 1 || d > MDAYS[m-1] ) ) {
		return BADDAYMONTH;
	}
	if ( m == 2 ) {
		int n = IsLeapYear( y ) ? 1 : 0;
		if (  d < 1U || d > 28U + n ) {
			return BADDAYMONTH;
		}
	}
	return DATEOK;
}

//---------------------------------------------------------------------------
// Quick hack for now
// ??? need to rationalise Parse/Validate ???
//---------------------------------------------------------------------------

Date :: Error Date :: Validate( const std::string & ds ) {
	int y, m, d;
	Parse( ds, y, m, d );
	return Validate( y, m, d ) ;
}

//---------------------------------------------------------------------------
// Difference in days between two dates in days
//---------------------------------------------------------------------------

int Date :: Diff( const Date & d1, const Date & d2 ) {
	return d1.mDays - d2.mDays;
}

//---------------------------------------------------------------------------
// Add number of days to date - number of days may be negative.
//---------------------------------------------------------------------------

Date Date :: Add( const Date & d1, int days ) {
	Date tmp = d1;
	tmp.mDays += days;
	return tmp;
}

//---------------------------------------------------------------------------
// Convert to string rep in  "yyyy-mm-dd" format
//---------------------------------------------------------------------------

string Date :: Str() const {
	std::ostringstream os;
	os << std::setfill( '0' )
		<< std::setw(4) << Year() << '-'
		<< std::setw(2) << Month() << "-"
		<< std::setw(2) << Day();
	return os.str();
}

//---------------------------------------------------------------------------
// Increment & assignment ops
//---------------------------------------------------------------------------

Date Date :: operator +=( int days ) {
	mDays += days;
	return * this;
}

Date Date :: operator -=( int days ) {
	mDays -= days;
	return * this;
}

Date Date :: operator ++ () {
	return (*this) += 1;
}

Date Date :: operator -- () {
	return (*this) -= 1;
}

//---------------------------------------------------------------------------
// Comparisons
//---------------------------------------------------------------------------

bool operator == ( const Date & d1, const Date & d2 ) {
	return Date::Diff( d1, d2 ) == 0;
}

bool operator != ( const Date & d1, const Date & d2 ) {
	return Date::Diff( d1, d2 ) != 0;
}

bool operator < ( const Date & d1, const Date & d2 ) {
	return Date::Diff( d1, d2 ) < 0;
}

bool operator > ( const Date & d1, const Date & d2 ) {
	return Date::Diff( d1, d2 ) > 0;
}

bool operator <= ( const Date & d1, const Date & d2 ) {
	return Date::Diff( d1, d2 ) <= 0;
}

bool operator >= ( const Date & d1, const Date & d2 ) {
	return Date::Diff( d1, d2 ) >= 0;
}

//---------------------------------------------------------------------------
// I/O operations
//---------------------------------------------------------------------------

std::ostream & operator << ( std::ostream & os, const Date & d ) {
	return os << d.Str();
}
std::istream & operator >> ( std::istream & is,	Date & d ) {
	string tmp;
	is >> tmp;
	d = Date( tmp );
	return is;
}

//---------------------------------------------------------------------------


} // end namespace

//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_date" );

DEFTEST( SimpleTest ) {
	Date d( "1953-08-19" );
	FAILNE( d.Day(), 19 );
	FAILNE( d.Month(), 8 );
	FAILNE( d.Year(), 1953 );
	FAILNE( d.ShortMonthName(), "Aug" );
}

DEFTEST( MathTest ) {
	Date d( "1953-08-19" );
	d += 3;
	FAILNE( d.Day(), 22 );
    d += 10;
	FAILNE( d.Day(), 1 );
	FAILNE( d.Month(), 9 );
}

DEFTEST( LeapTest ) {
	bool yes = Date::IsLeapYear( 2012 );
	FAILNE( yes, true );
	bool no = Date::IsLeapYear( 2013 );
	FAILNE( no, false );
}

#endif

//----------------------------------------------------------------------------

// end

