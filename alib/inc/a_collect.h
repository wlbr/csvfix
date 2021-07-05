//---------------------------------------------------------------------------
// a_collect.h
//
// Various utilities for dealing with templated collections.
// There is no matching .cpp file.
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_COLLECT_H
#define INC_A_COLLECT_H

#include "a_base.h"
#include <algorithm>
#include <iostream>
#include <vector>
#include <iterator>
#include <map>

//----------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// see if container contains value
//---------------------------------------------------------------------------

template <class C, class T>
bool Contains( const C & cntr, const T & val ) {
	return std::find( cntr.begin(), cntr.end(), val ) != cntr.end();
}

//---------------------------------------------------------------------------
// Get index of value in container. Intended for use with vectors but should
// work (possibly inefficiently) with other collections.
//---------------------------------------------------------------------------

template <class C, class T>
int  IndexOf( const C & cntr, const T & val ) {
	typename C::const_iterator it = std::find( cntr.begin(), cntr.end(), val );
	if ( it == cntr.end() ) {
		return -1;
	}
	else {
		return std::distance( cntr.begin(), it );
	}
}

//---------------------------------------------------------------------------
// Free all pointers in a collection by deleting them. The pointers must
// obviously have been created with op new.
//---------------------------------------------------------------------------

template <class C>
void FreePtrs( C & cntr ) {
	for ( typename C::iterator it = cntr.begin(); it != cntr.end(); ++it ) {
		delete * it;
	}
}

//----------------------------------------------------------------------------
// As above, but also clear collection
//----------------------------------------------------------------------------

template <class C>
void FreeClear( C & cntr ) {
	for ( typename C::iterator it = cntr.begin(); it != cntr.end(); ++it ) {
		delete * it;
	}
	cntr.clear();
}

//---------------------------------------------------------------------------
// Clear any kind of collection by assiging empty collection. Should work
// with non-collection types too.
//---------------------------------------------------------------------------

template <class C>
void Clear( C & c ) {
	c = C();
}

//----------------------------------------------------------------------------
// Get last item from a vector, or throw.
//----------------------------------------------------------------------------

template <class T>
const T & Last( const std::vector <T> & v ) {
	return v.at( v.size() - 1 );
}

//----------------------------------------------------------------------------
// Dump map on stream
//----------------------------------------------------------------------------

template <typename K, typename V>
void Dump( std::ostream & os, const std::map <K,V> & m ) {
	typename std::map<K,V>::const_iterator it = m.begin();
	while( it != m.end() ) {
		os << "[" << it->first << "] = [" << it->second << "]\n";
		it++;
	}
}

//---------------------------------------------------------------------------
// Dump non-associative collection on stream
//---------------------------------------------------------------------------

template <class C>
void Dump( std::ostream & os, const C & c,
								const std::string & nl = "\n" ) {
	typename C::const_iterator it = c.begin();
	while( it != c.end() ) {
		os << "[" << *it << "]" << nl;
		it++;
	}
}

//---------------------------------------------------------------------------
// Class to automatically free all pointers in vector when the manager
// goes out of scope.
// ??? probably get rid of this ???
//---------------------------------------------------------------------------

template <class T>
class VecMan {

	CANNOT_COPY( VecMan );

	public:

		VecMan( std::vector <T> & v )
			: mVec( v ) {}

		~VecMan() {
			for ( unsigned int i = 0; i < mVec.size(); i++ ) {
				delete mVec[i];
			}
			mVec.clear();
		}

	private:

		std::vector <T> & mVec;

};

//---------------------------------------------------------------------------
// Some operators that vector should support but doesn't.
//---------------------------------------------------------------------------

template <class T>
std::vector<T> & operator += ( std::vector<T> & v1,
								const std::vector <T> & v2 ) {
	v1.insert( v1.end(), v2.begin(), v2.end() );
	return v1;
}

template <class T>
std::vector<T> & Append( std::vector<T> & v1,
								const std::vector <T> & v2 ) {
	v1.insert( v1.end(), v2.begin(), v2.end() );
	return v1;
}

template <class T>
std::vector<T> operator + ( const std::vector<T> & v1,
											    const std::vector <T> v2 ) {
	std::vector <T> v( v1 );
	return v += v2;
}

//----------------------------------------------------------------------------

}  // end namespace


#endif
