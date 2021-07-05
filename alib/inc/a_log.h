//---------------------------------------------------------------------------
// a_log.h
//
// monostate logger for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_LOG_H
#define INC_A_LOG_H

#include "a_base.h"
#include <iosfwd>

namespace ALib {

//---------------------------------------------------------------------------

class Logger {

	public:

		enum Level { llAll = 0, llDebug = 1, llInfo = 2,
						llError = 4, llFatal = 8 };

		static void SetOutput( std::ostream & os );
		static void SetLevel( unsigned int lvl );

		static void Debug( const std::string & msg );
		static void Info( const std::string & msg );
		static void Error( const std::string & msg );
		static void Fatal( const std::string & msg );

		static void Write( Level lvl, const std::string & msg );

	private:

		static std::string Level2Str( Level lvl );
		static std::ostream * mOut;
		static unsigned int mLevel;

};


//---------------------------------------------------------------------------

}	// namespace

#endif

