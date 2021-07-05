//------------------------------------------------------------------------
// a_base.h
//
// Basic includes for all alib files. Contains configuration settings
// and generally useful macros and short functions.
//
// Copyright (C) 2008 Neil Butterworth
//------------------------------------------------------------------------

#ifndef INC_A_BASE_H
#define INC_A_BASE_H

#include "a_except.h"

//---------------------------------------------------------------------------
// The following pragmas turn off some annoying MSVC++ warnings
//---------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#pragma warning( disable : 4503 )
#endif

//---------------------------------------------------------------------------
// Just about everything uses these
//---------------------------------------------------------------------------

#include <string>
#include <vector>
#include <memory>

//---------------------------------------------------------------------------
// simple macro to prohibit copying
//---------------------------------------------------------------------------

#define CANNOT_COPY( cls )							\
	private:										\
		cls( const cls & );							\
		void operator=( const cls & )				\


//---------------------------------------------------------------------------
// I find reading some expressions containing ! operator difficult, so...
//---------------------------------------------------------------------------

namespace ALib {
	inline bool Not( bool val ) {
		return ! val;
	}
}


//---------------------------------------------------------------------------
// Everyone else has their version min/max, so why shouldn't I?
//---------------------------------------------------------------------------

namespace ALib {
	template <typename T> const T & Min( const T & t1, const T & t2 ) {
		return t1 < t2 ? t1 : t2;
	}

	template <typename T> const T & Max( const T & t1, const T & t2 ) {
		return t1 > t2 ? t1 : t2;
	}
}

#endif

