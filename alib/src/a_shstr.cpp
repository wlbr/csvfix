//---------------------------------------------------------------------------
// a_shstr.cpp
//
// Shared strings for alib. Shared strings are used when you use many
// copies of the same few strings.
//
// This software is licensed under the Apache 2.0 license. You should find
// a copy of the license in the file LICENSE.TXT. If this file is missing,
// a copy can be obtained from www.apache.org.
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include <map>
#include "a_shstr.h"
#include "a_except.h"
using std::string;

//---------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// Store contains strings with ref count, indexed by unique id.
//---------------------------------------------------------------------------

class SharedString::Store {

	public:

		Store() : mNextId(1) {
		}

		static unsigned int Add( const std::string & s );
		static void Remove( unsigned int id );
		static const std::string & Get( unsigned int i );
		static unsigned int Get( const std::string & s );

	private:


		struct Rep {
			std::string mStr;
			unsigned int mCount;
			Rep( const std::string & s ) : mStr(s), mCount(1) {}
		};

		typedef std::map <unsigned int, Rep> MapType;
		MapType mMap;
		unsigned int mNextId;
		static Store * mStore;
};

//---------------------------------------------------------------------------
// Single store instance
//---------------------------------------------------------------------------

SharedString::Store * SharedString::Store::mStore = 0;

//---------------------------------------------------------------------------
// Adds string to store if not already there
//---------------------------------------------------------------------------

SharedString :: SharedString( const string & s )  {
	mId = Store::Add( s );
}

//---------------------------------------------------------------------------
// Remove string from store if ref count falls to zero
//---------------------------------------------------------------------------

SharedString ::	~SharedString() {
	Store::Remove( mId );
}

//---------------------------------------------------------------------------
// Get copy of string
//---------------------------------------------------------------------------

string SharedString ::	Str() const {
	return Store::Get( mId );
}

//---------------------------------------------------------------------------
// Add string to store, returning unique id. If the string is already there,
// just increment the ref count.
//---------------------------------------------------------------------------

unsigned int SharedString::Store :: Add( const string & s ) {
	static bool needinit = true;
	if ( needinit ) {
		mStore = new SharedString::Store;
		needinit = false;
	}

	MapType::iterator it = mStore->mMap.begin();
	while( it != mStore->mMap.end() ) {
		if ( it->second.mStr == s ) {
			it->second.mCount++;
			return it->first;
		}
		++it;
	}
	Rep r( s );
	mStore->mMap.insert( std::make_pair( mStore->mNextId, r) );
	return 	mStore->mNextId++;
}

//---------------------------------------------------------------------------
// Remove from store by decrementing ref count. String is actually removed
// on;ly when ref count falls to zero.
//---------------------------------------------------------------------------

void SharedString::Store :: Remove( unsigned int id ) {
	MapType::iterator it = mStore->mMap.find( id );
	if ( it == mStore->mMap.end() ) {
		ATHROW( "Invalid id " << id << " in Remove()" );
	}
	if ( it->second.mCount == 1 ) {
		mStore->mMap.erase( it );
	}
	else {
		it->second.mCount--;
	}
}

//---------------------------------------------------------------------------
// Get string by unique id
//---------------------------------------------------------------------------

const string & SharedString::Store :: Get( unsigned int id ) {
	MapType::iterator it = mStore->mMap.find( id );
	if ( it == mStore->mMap.end() ) {
		ATHROW( "Invalid id " << id << " in Get()" );
	}
	return it->second.mStr;
}

//---------------------------------------------------------------------------

} // end namespace

// end
