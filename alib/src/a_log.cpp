//---------------------------------------------------------------------------
// a_log.cpp
//
// logging for alib
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_log.h"
#include "a_time.h"
#include <iostream>
#include <sstream>


using std::string;

namespace ALib {

//---------------------------------------------------------------------------
// Where to write log - logger does not own stream
//----------------------------------------------------------------------------

std::ostream * Logger::mOut = 0;

//----------------------------------------------------------------------------
// bitmask specifying what to log
//----------------------------------------------------------------------------

unsigned int Logger::mLevel = Logger::llAll;

//----------------------------------------------------------------------------
// set output stream to use
//----------------------------------------------------------------------------

void Logger :: SetOutput( std::ostream & os ) {
	mOut = & os;
}

//----------------------------------------------------------------------------
// set output level filtering
//----------------------------------------------------------------------------

void Logger :: SetLevel( unsigned int lvl ) {
	mLevel = lvl;
}

//----------------------------------------------------------------------------
// Translate logging  level to readable string
//----------------------------------------------------------------------------

string Logger :: Level2Str( Level lvl ) {
	switch ( lvl ) {
		case llDebug:	return "[DBG]";
		case llInfo:	return "[INF]";
		case llError:	return "[ERR]";
		case llFatal:	return "[FTL]";
		default: 		return "[???]";
	}
}


//----------------------------------------------------------------------------
// output functions for diffeent logging levels
//----------------------------------------------------------------------------

void Logger :: Debug( const std::string & msg ) {
	Write( Logger::llDebug, msg );
}
void Logger :: Info( const std::string & msg ) {
	Write( Logger::llInfo, msg );
}
void Logger :: Error( const std::string & msg ) {
	Write( Logger::llError, msg );
}
void Logger :: Fatal( const std::string & msg ) {
	Write( Logger::llFatal, msg );
	throw ALib::Exception( msg );
}

//----------------------------------------------------------------------------
// general purpose output to log
//----------------------------------------------------------------------------

void Logger :: Write( Level lvl, const string & msg ) {
	if ( mOut == 0 ) {
		mOut = & std::cerr;
	}
	if ( mLevel == llAll || ( lvl & mLevel ) ) {
		(*mOut) << '[' << Time::TimeStampStr() << ']'
		        << Level2Str( lvl )
		        << '[' << msg << ']'
		        << std::endl;
	}
}


//----------------------------------------------------------------------------

} // end namespace

//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
#include <sstream>
using namespace ALib;
using namespace std;

DEFSUITE( "a_log" );

DEFTEST( Simple ) {
	ostringstream os;
	Logger::SetOutput( os );
	Logger::Info( "some info" );
	string line = os.str();
	FAILIF( line.find( "some info" ) == STRNPOS );
}

#endif

//----------------------------------------------------------------------------


// end
