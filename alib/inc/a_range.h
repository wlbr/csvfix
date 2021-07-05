//---------------------------------------------------------------------------
// a_range.h
//
// ranges for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_ALIB_RANGE_H
#define INC_ALIB_RANGE_H

#include "a_base.h"
#include "a_str.h"

namespace ALib {

//---------------------------------------------------------------------------

template <typename T> class Range {

	public:

		Range( const T & t1, const T& t2 )
			: mBegin( t1 ), mEnd( t2 ) {
			if ( mBegin > mEnd ) {
				ATHROW( "invalid range" );
			}
		}

		Range( const std::string r, char sep = ':' ) {
			std::istringstream is( r );
			char c;
			if ( ! (is >> mBegin >> c >> mEnd )
			     || c != sep
			     || mBegin > mEnd  ) {
			     ATHROW( "Invalid range" );
			}
		}

		bool Contains( const T & t ) const {
			return t >= mBegin && t <=  mEnd;
		}

		const T & Begin() const {
			return mBegin;
		}

		const T & End() const {
			return mEnd;
		}

		int ExpandInto( std::vector <T> & v ) const {
			T t = mBegin;
			do {
				v.push_back( t++ );
			} while( t <= mEnd );
			return v.size();
		}

		std::ostream & DumpOn( std::ostream & os ) {
			return os << mBegin << ':' << mEnd;
		}

	private:

		T mBegin, mEnd;

};

//----------------------------------------------------------------------------

template <typename T >
std::ostream & operator << ( std::ostream & os, const Range <T> & r ) {
	return r.DumpOn( os );
}



} // namespace ALib

#endif

