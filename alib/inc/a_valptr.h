//---------------------------------------------------------------------------
// a_valptr.h
//
// Value pointer for us as class member. Based on exposition in
// "Exceptional C++". The main feature is that if the pointer is copied
// so is the thing pointed to, so it can be used in std:: collections.
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_VALPTR_H
#define INC_A_VALPTR_H

#include "a_base.h"
#include "a_except.h"


namespace ALib {

//---------------------------------------------------------------------------

template  <class T> class ValPtr {

	public:

		explicit ValPtr( T * p = 0 ) : mPtr( p ) {
		}

		ValPtr( const ValPtr & vp )
			: mPtr( vp.mPtr ? new T( * vp.mPtr ) : 0 ) {
		}

		~ValPtr() {
			delete mPtr;
		}

		ValPtr & operator =( const ValPtr & vp ) {
			ValPtr t( vp );
			Swap( t );
			return * this;
		}

		ValPtr & operator =( T * tp ) {
			delete mPtr;
			mPtr = tp;
			return * this;
		}

		void Swap( ValPtr & vp ) {
			swap( mPtr, vp.mPtr );
		}

		T & operator * () const {
			if ( mPtr == 0 ) {
				ATHROW( "ValPtr::operator*() called on null pointer" );
			}
			return * mPtr;
		}

		T * operator -> () const {
			if ( mPtr == 0 ) {
				ATHROW( "ValPtr::operator->() called on null pointer" );
			}
			return mPtr;
		}

		const T * Get() const {
			return mPtr;
		}

		T * Get() {
			return mPtr;
		}

		T * Release() {
			T * tmp( mPtr );
			mPtr = 0;
			return tmp;
		}

	private:

		T * mPtr;

};

//---------------------------------------------------------------------------

}	// namespace

#endif

