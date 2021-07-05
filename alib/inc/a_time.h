//---------------------------------------------------------------------------
// a_time.h
//
// time related stuff  for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_TIME_H
#define INC_A_TIME_H

#include "a_base.h"


namespace ALib {

//------------------------------------------------------------------------

class Time {

	public:

		Time( time_t t );

		time_t AsTimeT() const;
		std::string TimeStamp() const;
		std::string Str() const;

		static std::string TimeStampStr();
		static Time Now();

	private:

		time_t mTime;
};


//----------------------------------------------------------------------------

}  // namespace


#endif

