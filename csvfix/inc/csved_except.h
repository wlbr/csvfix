//---------------------------------------------------------------------------
// csved_except.h
//
// CSVED exceptions
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_EXCEPT_H
#define INC_CSVED_EXCEPT_H

#include "a_except.h"

namespace CSVED {

//---------------------------------------------------------------------------
// All CSVED exceptions are of this type
//---------------------------------------------------------------------------

class Exception : public ALib::Exception {
	public:
		Exception( const std::string & msg = "" )
			: ALib::Exception( msg ) {}
};


//---------------------------------------------------------------------------
// Handles all exceptions that make it to main()
//---------------------------------------------------------------------------

class ExceptionHandler {

	public:

		static int HandleMyError( const CSVED::Exception & ex );
		static int HandleOtherError( const std::exception & ex );
		static int HandleUnknownError();
};

//---------------------------------------------------------------------------
// Macro to throw a CSVT exception with stream-based message formatting
//------------------------------------------------------------------------

#define CSVTHROW( msg )												\
{																	\
	std::ostringstream os_;											\
	os_<< msg;														\
	throw CSVED::Exception( os_.str() );							\
}																	\

//---------------------------------------------------------------------------

}	// end namespace

#endif
