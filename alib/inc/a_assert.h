//---------------------------------------------------------------------------
// a_assert.h
//
// assertions for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_ASSERT_H
#define INC_A_ASSERT_H

#include "a_except.h"
#include "a_str.h"

//---------------------------------------------------------------------------
// Begin ALib stuff
//---------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// ALib Assertions throw this
//---------------------------------------------------------------------------

class AssertFailed : public Exception {

	public:

		AssertFailed( const std::string & msg,
						int line, const std::string & file )
			: Exception( msg, line, file ) {}

};

//---------------------------------------------------------------------------
// Assert macro
//---------------------------------------------------------------------------

#define AASSERT( cond ) {													\
	if ( ! (cond) ) {														\
		throw AssertFailed( "Assertion failed: " + std::string( #cond ),	\
								__LINE__, __FILE__ );						\
	}																		\
}																			\


//---------------------------------------------------------------------------
// Assertion  reporting macro
//---------------------------------------------------------------------------

#define CATCH_AASSERT														\
	catch( const ALib::AssertFailed & a ) {									\
		std::cerr << "Assertion failed: " <<	 a.Msg() << "\n";			\
		std::cerr << "Line: " << a.Line() << "\n";							\
		std::cerr << "File: " << a.File() << std::endl;						\
	}																		\
	
	 			
//---------------------------------------------------------------------------
// End Alib stuff
//---------------------------------------------------------------------------

}

#endif

