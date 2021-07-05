//---------------------------------------------------------------------------
// a_except.h
//
// various alib exception handling stuff
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_EXCEPT_H
#define INC_A_EXCEPT_H

#include "a_base.h"
#include <exception>
#include <sstream>

//------------------------------------------------------------------------
// begin ALib stuff
//------------------------------------------------------------------------

namespace ALib {


//------------------------------------------------------------------------
// The only exception thrown directly by alib
//------------------------------------------------------------------------

class Exception : public std::exception {

	public:

		Exception( const std::string & msg = "" );
		Exception( const std::string & msg, int line,
						const std::string & file );

		~Exception() throw();

		const char *what() const throw();
		const std::string & Msg() const;

		int Line() const;
		const std::string & File() const;

	private:

		std::string mMsg, mFile;
		int mLine;
};

//------------------------------------------------------------------------
// Macro to throw an alib exception with message formatting.
// Remember macro is not in ALib namespace!
//------------------------------------------------------------------------

#define ATHROW( msg )												\
{																	\
	std::ostringstream os;											\
	os << msg;														\
	throw ALib::Exception( os.str(), __LINE__, __FILE__ );			\
}																	\

#define ATHROWE( except, msg )										\
{																	\
	std::ostringstream os;											\
	os << msg;														\
	throw except( os.str(), __LINE__, __FILE__ );					\
}																	\

//----------------------------------------------------------------------------
// Macro to implement a named exception type based on Exception
//----------------------------------------------------------------------------

#define IMPL_EXCEPT( exname )										\
	class exname : public ALib::Exception {							\
		public:														\
			exname( const std::string & msg)  						\
				: ALib::Exception( msg ) {}							\
			exname( const std::string & msg, int line,				\
						const std::string & file )					\
				: ALib::Exception( msg, line, file ) {}				\
	};																\


//------------------------------------------------------------------------
// end ALib stuff
//------------------------------------------------------------------------

}

#endif
