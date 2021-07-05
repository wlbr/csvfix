//----------------------------------------------------------------------------
// a_strscan.h
//
// simple string scanner
//
// Copyright (C) 20010 Neil Butterworth
//----------------------------------------------------------------------------


#ifndef INC_ALIB_STRSCAN_H
#define INC_ALIB_STRSCAN_H

#include <string>

namespace ALib {

//----------------------------------------------------------------------------

class StrScanner {

	public:

		static const int EOS = -1;

		StrScanner( const std::string & s = "" );

		int Next();
		int Current() const;
		int Peek( unsigned int  n = 0 ) const;

	private:

		void CheckNextCalled() const;

		std::string mStr;
		unsigned int mPos;
		int  mCurrent;

};

//----------------------------------------------------------------------------

} // namespace

#endif
