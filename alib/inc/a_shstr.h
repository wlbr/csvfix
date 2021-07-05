//---------------------------------------------------------------------------
// a_shstr.h
//
// Shared strings for alib. Shared strings are used when you need many
// copies of the same few strings.
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_SHSTR_H
#define INC_A_SHSTR_H

#include "a_base.h"

namespace ALib {

//---------------------------------------------------------------------------

class SharedString {

	CANNOT_COPY( SharedString );

	public:

		class Store;

		SharedString( const std::string & s );
		~SharedString();

		std::string Str() const;


	private:

		unsigned int mId;
};

//---------------------------------------------------------------------------

}	// namespace

#endif

