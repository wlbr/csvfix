//---------------------------------------------------------------------------
// a_nameval.cpp
//
// name/value pair stuff for alib
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_nameval.h"
#include <fstream>
using std::string;
using std::vector;

namespace ALib {

//------------------------------------------------------------------------
// do nothing ctor & virtual dtor for base class
//---------------------------------------------------------------------------

NVPSource :: NVPSource() {
}

NVPSource :: ~NVPSource() {
}

//---------------------------------------------------------------------------
// Helper to parse a line in the form "name = value" int name & value
//---------------------------------------------------------------------------

bool NVPSource :: Parse( const string & line, string & name, 
									string & val ) const {
	vector <string> tmp;
	if ( ! ALib::Split( line, '=', tmp ) ) {
		return false;
	}
	name = ALib::Trim( tmp[0] );
	val = ALib::Trim( tmp[1] );
	return true;
}

//---------------------------------------------------------------------------
// Wrap dictionary in NVP interface
//---------------------------------------------------------------------------

NVPDictSource :: NVPDictSource() {
}

NVPDictSource :: ~NVPDictSource() {
	Clear();
}

void NVPDictSource :: Clear() {
	mDict.Clear();
}

bool NVPDictSource :: Contains( const std::string & name ) const {
	return mDict.Contains( name );
}

string NVPDictSource :: Value( const string & name ) const {
	return mDict.Get( name );
}

string NVPDictSource :: Value( const string & name,
									const string & defval ) const {
	const string * s = mDict.GetPtr( name );
	if ( s != 0 ) {
		return * s;
	}
	else {
		return defval;
	}
}

unsigned int NVPDictSource :: Names( vector <string> & names ) const {
	names.clear();
	mDict.GetNames( names );
	return names.size();
}

void NVPDictSource :: Add( const string & name, const string & value ) {
	mDict.Add( name, value );
}

void NVPDictSource :: Add( const string & line ) {
	string name, val;
	if ( ! Parse( line, name, val ) ) {
		ATHROW( "Invalid name/value pair: " << line );
	}
	Add( name, val );
}

//---------------------------------------------------------------------------
// File source loads from text file containing lines in form "name = value"
// Ignores blank lines and those beginnig with '#'
//---------------------------------------------------------------------------

NVPFileSource :: NVPFileSource() {
}

NVPFileSource :: NVPFileSource( const string & filename ) {
	LoadFrom( filename );
}

		
void NVPFileSource :: LoadFrom( const std::string & filename ) {
	Clear();
	std::ifstream ifs( filename.c_str() );
	if ( ! ifs.is_open() ) {
		ATHROW( "Cannot open " << filename << " for input" );
	}
	string line;
	while( std::getline( ifs, line ) ) {
		if ( ALib::IsEmpty( line ) || line[0] == '#' ) {
			continue;
		}
		Add( line );
	}
	ifs.close();
}

//---------------------------------------------------------------------------
// Load from string containing "name=value" pairs separated by 'sep'
//---------------------------------------------------------------------------

NVPStringSource :: NVPStringSource() {
}

NVPStringSource :: NVPStringSource( const string & s, char sep )  {
	LoadFrom( s, sep );
}
		
void NVPStringSource :: LoadFrom( const string & s, char sep ) {
	vector <string> tmp;
	Split( s, sep, tmp );
	for ( unsigned int i = 0; i < tmp.size(); i++ ) {
		if ( ! ALib::IsEmpty( tmp[i] ) ) {
			Add( tmp[i] );
		}
	}
}


//---------------------------------------------------------------------------

} // end namespace

// end

