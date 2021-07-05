//---------------------------------------------------------------------------
// a_inifile.cpp
//
// inifile reader for alib
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_inifile.h"
#include "a_str.h"
#include <fstream>

using std::string;
using std::vector;

namespace ALib {

//----------------------------------------------------------------------------
// create inifile object (not disk file), populating if filename provided
//----------------------------------------------------------------------------

IniFile :: IniFile( const string & filename ) {
	if ( filename != "" ) {
		Read( filename );
	}
}

//----------------------------------------------------------------------------
// do clear
//----------------------------------------------------------------------------

IniFile :: ~IniFile() {
	Clear();
}

//----------------------------------------------------------------------------
// remove entries & reset filename
//----------------------------------------------------------------------------

void IniFile :: Clear() {
	mMap.clear();
	mFileName = "";
}

//----------------------------------------------------------------------------
// helper to check if line is comment or empty
// note line will alreadhy have been trimmed
//----------------------------------------------------------------------------

static bool IsComment( const string & line ) {
	if ( line == "" || line[0] == ';' || line[0] == '#' ) {
		return true;
	}
	else {
		return false;
	}
}

//----------------------------------------------------------------------------
// helper to get section name - an empty name is ok
//----------------------------------------------------------------------------

static bool GetSection( const string & line, string & sect ) {
	if ( line[0] == '[' ) {
		if ( StrLast( line ) == ']' ) {
			sect = Trim( line.substr( 1, line.length() - 2 ) );
			return true;
		}
		else {
			ATHROW( "Invalid section: " << sect );
		}
	}
	else {
		return false;
	}
}

//----------------------------------------------------------------------------
// read named inifile, replacing any existing content
//----------------------------------------------------------------------------

void IniFile :: Read( const string & filename ) {
	Clear();
	mFileName = filename;
	std::ifstream ifs( filename.c_str() );
	if ( ! ifs.is_open() ) {
		ATHROW( "Cannot open config file: " << filename );
	}
	string line, sect;
	while( std::getline( ifs, line ) ) {
		line = Trim( line );
		if ( IsComment( line ) ) {
			continue;
		}
		else if ( GetSection( line, sect ) ) {
			if ( HasSection( sect ) ) {
				ATHROW( "Duplicate section name: " << sect );
			}
		}
		else {
			vector <string> vs;
			if ( Split( line, '=', vs ) != 2 ) {
				ATHROW( "Invalid setting: " << line );
			}
			if ( HasSetting( sect, vs[0] ) ) {
				ATHROW( "Duplicate setting " << vs[0] << " in section ["
							<< sect << "]" );
			}
			Add( sect, vs[0], vs[1] );
		}
	}
}

//----------------------------------------------------------------------------
// get file name last loaded - may be empty
//----------------------------------------------------------------------------

const string & IniFile :: FileName() const {
	return mFileName;
}

//----------------------------------------------------------------------------
// add entry, replacing any existing entry
//----------------------------------------------------------------------------

void IniFile :: Add( const string & section,
						const string & name,
						const string & value )  {

	Key k( section, name );
	MapType::iterator it = mMap.find( k );
	if ( it != mMap.end() ) {
		mMap.erase( it );
	}
	mMap.insert( std::make_pair( k, value ) );
}

//----------------------------------------------------------------------------
// see if named section exist
//----------------------------------------------------------------------------

bool IniFile :: HasSection( const string & section ) const {
	MapType::const_iterator it = mMap.begin();
	while( it != mMap.end() ) {
		if ( Equal( it->first.mSection, section ) ) {
			return true;
		}
		++it;
	}
	return false;
}


//----------------------------------------------------------------------------
// see if named section & value name exists
//----------------------------------------------------------------------------

bool IniFile :: HasSetting( const string & section, const string & name ) const {
	Key k( section, name );
	return mMap.find( k ) != mMap.end();
}

//----------------------------------------------------------------------------
// get value or empty string if not there
//----------------------------------------------------------------------------

string IniFile :: Value( const string & section, const string & name ) const {
	Key k( section, name );
	MapType::const_iterator it = mMap.find( k );
	if ( it == mMap.end() ) {
		return "";
	}
	else {
		return it->second;
	}
}

//----------------------------------------------------------------------------
// construct key for entry
//----------------------------------------------------------------------------

IniFile::Key :: Key( const std::string & sect, const std::string & name )
	: mSection( sect ), mName( name ) {
}

//----------------------------------------------------------------------------
// do case insensitive comparison of two keys
//----------------------------------------------------------------------------

bool IniFile::Key :: operator < ( const IniFile::Key & e ) const {
	if ( Cmp( this->mSection, e.mSection, IgnoreCase ) < 0 ) {
		return true;
	}
	else if ( Cmp( this->mSection, e.mSection, IgnoreCase ) == 0
	          && Cmp( this->mName, e.mName, IgnoreCase ) < 0 ) {
		return true;
	}
	else {
		return false;
	}
}

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

#ifdef ALIB_TEST

#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_inifile" );

DEFTEST( Simple ) {
	IniFile ini;
	FAILIF( ini.HasSection( "stuff" ) );
	ini.Add( "stuff", "meaning", "42" );
	FAILIF( ! ini.HasSection( "stuff" ) );
	string val = ini.Value( "stuff", "meaning" );
	FAILNE( val, "42" );
	val = ini.Value( "stuff", "bad" );
	FAILNE( val, "" );
	val = ini.Value( "stuff", "MEANING" );
	FAILNE( val, "42" );
	ini.Add( "stuff", "colour", "purple" );
	val = ini.Value( "stuff", "colour" );
	FAILNE( val, "purple" );
	ini.Add( "other", "colour", "green" );
	val = ini.Value( "other", "colour" );
	FAILNE( val, "green" );
}

DEFTEST( ReadFile ) {
	IniFile ini( "test/test.ini" );
	FAILIF( ! ini.HasSection( "values" ) );
	string val = ini.Value( "values", "meaning" );
	FAILNE( val, "42" );
}


#endif

// end

