//---------------------------------------------------------------------------
// a_date.h
//
// date handling for alib
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_DATE_H
#define INC_A_DATE_H


#include "a_base.h"
#include <iostream>

namespace ALib {

//---------------------------------------------------------------------------
/// Provides basic date handling.
//----------------------------------------------------------------------------

class Date {

	public:

		/// construct from ISO date string in form yyyy-mm-dd
		Date( const std::string & s = "" );

		/// construct from year, month, day - month and day are 1 based
		Date( int year, int month, int day );

		/// month day in range 1-31
		int Day() const;

		/// month in range 1-12
		int Month() const;

		/// 4-digit year
		int Year() const;

		/// day of year in range 1-366
		int DayOfYear() const;

		/// day of week in range 1-7
		int DayOfWeek() const;

		/// Full month name in English e.g. August
		std::string MonthName() const;

		/// Abreviated month name e.d. Aug.
		std::string ShortMonthName() const;

		/// Full day name in English e.g. Tuesday
		std::string DayName() const;

		/// Abreviated day name e.g. Tue
		std::string ShortDayName() const;

		// Difference between two dates as number of days
		static int Diff( const Date & d1, const Date & d2 );

		/// add days to date giving new date - days may be negative
		static Date Add( const Date & d1, int days );

		/// Add days to date
		Date operator +=( int days );
		Date operator -=( int days );

		// Inc date by one day
		Date operator ++ ();

		/// Decrement date by one day
		Date operator -- ();

		/// Fet today's date using the std::time() function
		static Date Today();

		/// error types
		enum Error {DATEOK, BADDAY, BADMONTH, BADDAYMONTH, BADYEAR };

		/// validate dates - formats are as for Date constructors
		static Error Validate( unsigned int y, unsigned int m, unsigned int d );
		static Error Validate( const std::string & ds );

		/// Parse date in ISO format into year, month, date values
		static bool Parse( const std::string & s, int & y, int & m, int & d );

		/// Is 4-digit year a leap year?
		static bool IsLeapYear( int y );

		/// Is date in  a leap year?
		bool IsLeapYear() const;

		/// Convert string to ISO yyyy-mm-dd form
		std::string Str() const;

	private:


		unsigned int mDays;

};

//---------------------------------------------------------------------------
// Various binary ops
//---------------------------------------------------------------------------

bool operator == ( const Date & d1, const Date & d2 );
bool operator != ( const Date & d1, const Date & d2 );
bool operator < ( const Date & d1, const Date & d2 );
bool operator > ( const Date & d1, const Date & d2 );
bool operator <= ( const Date & d1, const Date & d2 );
bool operator >= ( const Date & d1, const Date & d2 );

//---------------------------------------------------------------------------
/// I/O operators.  The input & output formats are ISO yyyy-mm-dd.
//---------------------------------------------------------------------------

std::ostream & operator << ( std::ostream & os, const Date & d );
std::istream & operator >> ( std::istream & is,	Date & d );


//------------------------------------------------------------------------

}	// end namespace

#endif

