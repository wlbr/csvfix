//---------------------------------------------------------------------------
// a_time.cpp
//
// time related stuff for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_time.h"
#include <iostream>
#include <iomanip>
#include <time.h>
#include <sstream>

using std::string;

namespace ALib {

//----------------------------------------------------------------------------
// Construct from seconds since epoch start.
//----------------------------------------------------------------------------

Time :: Time( time_t t ) : mTime( t) {
}


//----------------------------------------------------------------------------
// Get current time
//----------------------------------------------------------------------------

Time Time :: Now() {
	return Time( time( 0 ) );
}


//----------------------------------------------------------------------------
// Time in hh:mm:ss format
//----------------------------------------------------------------------------

string Time :: Str() const {
	struct tm * t = localtime( & mTime );
	std::ostringstream os;
	os << std::setfill('0') << std::setw(2) << t->tm_hour
		<< ':' << std::setfill('0') << std::setw(2) << t->tm_min
		<< ':' << std::setfill('0') << std::setw(2) << t->tm_sec;
	return os.str();
}

//----------------------------------------------------------------------------
// Convert to yyy-mm-dd hh:mm:ss timestamp
//----------------------------------------------------------------------------

std::string Time :: TimeStamp() const {
	struct tm * t = localtime( & mTime );
	std::ostringstream os;
	os << t->tm_year + 1900
		<< '-' << std::setfill('0') << std::setw(2) << t->tm_mon + 1
		<< '-' << std::setfill('0') << std::setw(2) << t->tm_mday
		<< ' ' << std::setfill('0') << std::setw(2) << t->tm_hour
		<< ':' << std::setfill('0') << std::setw(2) << t->tm_min
		<< ':' << std::setfill('0') << std::setw(2) << t->tm_sec;
	return os.str();
}

//----------------------------------------------------------------------------
// Produce timestamp string for current time. Needed for some older code.
//----------------------------------------------------------------------------

string Time :: TimeStampStr() {
	return Time::Now().TimeStamp();
}

//----------------------------------------------------------------------------

}  // namespace


// end
