//---------------------------------------------------------------------------
// a_slice.h
//
// Array slice for alib. This uses (partial) standard library naming
// conventions in order to to see if I still dislike them, and also because
// Slice is supposed to work just like the vector it is a slice of.
//
// ??? constness is not right ???
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_SLICE_H
#define INC_A_SLICE_H

#include "a_base.h"
#include <vector>

namespace ALib {

//----------------------------------------------------------------------------
// This class provides views on slices of a vector, which may be obtained
// by going through intermediate slices. Slices maintain a pointer to the
// vector theyrefer to, but do not own the vector. Thus no copy ctor, dtor or
// assignment operator is needed.
//----------------------------------------------------------------------------

template <typename T>
class Slice {

	public:

		typedef typename std::vector<T> VecType;
		typedef typename VecType::iterator iterator;
		typedef typename VecType::const_iterator const_iterator;

		Slice() : mVec( 0 ), mStart(0), mSize(0) {}

		Slice( const VecType & v )
			: mVec( & v ), mStart(0), mSize( v.size() ) {}

		Slice( const VecType & v, unsigned int start, size_t n  )
			: mVec( & v ), mStart( start ), mSize( n ) {
			if (  start + n > v.size() ) {
				ATHROW( "Invalid slice start and/or size" );
			}
		}

		Slice( const Slice<T> & s, unsigned int start, size_t n  )
			: mVec( s.mVec ), mStart( s.mStart + start ), mSize( n ) {

			if (  mVec == 0 || start + n > s.size() ) {
				ATHROW( "Invalid slice start and/or size" );
			}
		}

		const T & operator[]( unsigned int i ) const {
			return mVec->operator[]( Index( i ) );
		}

		T & operator[]( unsigned int i ) {
			return const_cast <VecType *>(mVec)->operator[]( Index(i) );
		}

		const T & at( unsigned int i ) const {
			Check();
			if ( Index(i) >= mStart + mSize ) {
				ATHROW( "Invalid slice index: " << i );
			}
			return mVec->at( Index(i) );
		}

		T & at( unsigned int i ) {
			Check();
			return const_cast <VecType *>(mVec)->at( Index(i) );
		}

		bool is_assigned() const {
			return mVec != 0;
		}

		size_t size() const {
			return mSize;
		}

		const_iterator begin() {
			Check();
			return mVec	->begin() + mStart;
		}

		const_iterator end() {
			Check();
			return begin() + mSize;
		}


	private:

		unsigned int Index( unsigned int i ) const {
			return i + mStart;
		}

		void Check() const {
			if ( ! is_assigned() ) {
				ATHROW( "Attempt to use unassigned ALib::Slice" );
			}
		}

		const VecType * mVec;		// vector we refer to
		unsigned int mStart;		// our start pos in the vector
		size_t mSize;				// length from start

};

//----------------------------------------------------------------------------

}	// namespace
#endif

