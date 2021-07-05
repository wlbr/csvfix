//---------------------------------------------------------------------------
// a_dict.h
//
// Dictionary class maps strings to another type. This is slightly easier
// to use than a normal map but doesn't provide iterator support.
//
// Features:
//
//	- Duplicate key values are not allowed.
//	- Keys are compared with ALib::Less()
//	- Keys ignore case by default
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_DICT_H
#define INC_A_DICT_H

#include "a_base.h"
#include <iostream>
#include <map>
#include "a_except.h"
#include "a_str.h"

namespace ALib {

//---------------------------------------------------------------------------

template <class TYPE, CaseSensitive CS = IgnoreCase> class Dictionary {

	public:

		typedef std::map <std::string, TYPE, Less<CS> > MapType;

		// does dict contain any entries?
		bool IsEmpty() {
			return mMap.size() == 0;
		}

		// remove all entries
		void Clear() {
			mMap.clear();
		}

		// get underlying map

		MapType & Map() {
			return mMap;
		}

		const MapType & Map() const{
			return mMap;
		}

		// does dict contain named entry?
		bool Contains( const std::string & name ) const {
			return mMap.find( name ) != mMap.end();
		}

		// populate vector with all names in dictionary
		void GetNames( std::vector <std::string> & names ) const {
			typename MapType::const_iterator it = mMap.begin();
			while( it != mMap.end() ) {
				names.push_back( it->first );
				++it;
			}
		}

		// get commands that begin with ab
		void GetAbbreviations( const std::string & ab, std::vector <std::string> & names ) const {
			typename MapType::const_iterator it = mMap.begin();
			while( it != mMap.end() ) {
				std::string n = it->first.substr( 0, ab.size() );
				if ( ALib::Equal( ab, n ) ) {
					names.push_back( it->first );
				}
				++it;
			}
		}

		// return pointer to entry value if it exists, NULL if not
		const TYPE * GetPtr( const std::string & name ) const {
			typename MapType::const_iterator it = mMap.find( name );
			if ( it == mMap.end() ) {
				return 0;
			}
			else {
				return & it->second;
			}
		}

		// get entry value or throw exception if dosn't exist
		const TYPE & Get( const std::string & name ) const {
			const TYPE * p = GetPtr( name );
			if ( p == 0 ) {
				ATHROW( "Dictionary key " << SQuote(name) << " not found" );
			}
			return * p;
		}

		// get entry value or default if dosn't exist
		const TYPE & Get( const std::string & name,
							const std::string & def ) const {
			const TYPE * p = GetPtr( name );
			if ( p == 0 ) {
				return def;
			}
			else {
				return * p;
			}
		}

		// remove named entry or do nothing if does not exist
		void Remove( const std::string & name ) {
			mMap.erase( name );
		}

		// add name/value to dictionary - fails on duplicate key
		bool Add( const std::string & name,  const TYPE & val ) {
			return mMap.insert( std::make_pair( name, val ) ).second;
		}

		// replace adds if name does not exist
		void Replace( const std::string & name,  const TYPE & val ) {
			typename MapType::iterator it = mMap.find( name );
			if ( it == mMap.end() ) {
				mMap.insert( std::make_pair( name, val ) );
			}
			else {
				it->second = val;
			}
		}

		// get all keys that have a value
		int KeysForValue( std::vector <std::string> & keys, const TYPE & val ) {
			int n = 0;
			for ( typename MapType::iterator it = mMap.begin();
					it != mMap.end(); ++it ) {
				if ( it->second == val ) {
					keys.push_back( it->first );
					n++;
				}
			}
			return n;
		}


		// add name/value pair in form "name=value"
		// only works for string->string dicts
		bool AddNVP( const std::string & nv ) {
			STRPOS epos = nv.find( '=' );
			if ( epos == STRNPOS ) {
				ATHROW( "Invalid name/value pair: " << nv );
			}
			std::string name = nv.substr( 0, epos );
			std::string value = nv.substr( epos + 1 );
			return Add( Trim(name), Trim(value) );
		}

		// dump dictionary on stream
		// requires values to be streamable
		void DumpOn( std::ostream & os ) const {
			typename MapType::const_iterator it = mMap.begin();
			while( it != mMap.end() ) {
				os << it->first << "=" << it->second << std::endl;
				it++;
			}
		}

	private:

		MapType mMap;

};

//---------------------------------------------------------------------------

}	// end namespace

#endif

