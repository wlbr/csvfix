//---------------------------------------------------------------------------
// a_file.cpp
//
// file utilities for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_assert.h"
#include "a_file.h"
#include "a_except.h"
#include "a_str.h"
#include "a_collect.h"
// We assume support for dirent - true for MinGW and Linux.
#include <dirent.h>


using std::string;
using std::vector;

//---------------------------------------------------------------------------

namespace ALib {


//----------------------------------------------------------------------------
// Wrapper roun dirent functions. This gives some minimal portability as
// it's supported by MinGW and on UNIX.
//----------------------------------------------------------------------------

class DirEnt {

	public:

		DirEnt();
		DirEnt( const std::string & dirname );
		~DirEnt();

		void Clear();
		void Populate( const std::string & dir );

		unsigned int Size() const;
		string Filename( unsigned int  i ) const;

		static bool Exists( const std::string & dir );

	private:

		vector <struct dirent *> mDir;
};

//----------------------------------------------------------------------------
// Ctors & dtor
//----------------------------------------------------------------------------


DirEnt :: DirEnt() {
}

DirEnt :: DirEnt( const std::string & dirname ) {
	Populate( dirname );
}

DirEnt :: ~DirEnt() {
	Clear();
}

//----------------------------------------------------------------------------
// Populate with files from namesd directory. If directory does not exist
// or cannot be read then throw.
//----------------------------------------------------------------------------

void DirEnt ::Populate( const std::string & dirname ) {
	Clear();
	DIR * dir = opendir( dirname.c_str() );
	if ( dir == 0 ) {
		ATHROW( "Could not open directory " << dirname );
	}

	while( struct dirent * d = readdir( dir ) ) {
		struct dirent * dd = new struct dirent;
		* dd = * d;
		mDir.push_back( dd );
	}
	closedir( dir );
}

//----------------------------------------------------------------------------
// Remove all entries
//----------------------------------------------------------------------------

void DirEnt :: Clear() {
	FreeClear( mDir );
}

//----------------------------------------------------------------------------
// Accessors. Filename accessor will currntly also give directories.
//----------------------------------------------------------------------------

unsigned int DirEnt :: Size() const {
	return mDir.size();
}

string DirEnt :: Filename( unsigned int  i ) const {
	return mDir.at(i)->d_name;
}

//----------------------------------------------------------------------------
// See if dir exists & can be opened
//----------------------------------------------------------------------------

bool DirEnt :: Exists( const std::string & dirname ) {
	DIR * dir = opendir( dirname.c_str() );
	if ( dir ) {
		closedir( dir );
		return true;
	}
	else {
		return false;
	}
}

//---------------------------------------------------------------------------
// Empty filename
//---------------------------------------------------------------------------

Filename :: Filename()  {
}

//---------------------------------------------------------------------------
// Filename with backslashes normalised to forward slashes
//---------------------------------------------------------------------------

Filename :: Filename( const string & fname )
	: mName( fname ) {
	unsigned int n = mName.size();
	for ( unsigned int i = 0; i < n; i++ ) {
		if ( mName[i] == '\\' ) {
				mName[i] = '/';
		}
	}
}

//---------------------------------------------------------------------------
// Whole filename
//---------------------------------------------------------------------------

const string & Filename :: Str() const {
	return mName;
}

//---------------------------------------------------------------------------
// Base (drive and directory removed) name
//---------------------------------------------------------------------------

string Filename :: Base() const {
	STRPOS pos = mName.find_last_of( "/" );
	if ( pos == STRNPOS ) {
		return mName;
	}
	else {
		return mName.substr( pos + 1 );
	}
}

//---------------------------------------------------------------------------
// Base without extension
//---------------------------------------------------------------------------

string Filename :: BaseNoExt() const {
	string s = Base();
	STRPOS pos = s.find_last_of( "." );
	if ( pos == STRNPOS ) {
		return s;
	}
	else {
		return s.substr( 0, pos );
	}
}


//---------------------------------------------------------------------------
// Base extension, with dot, or empty string if no extension
//---------------------------------------------------------------------------

string Filename :: Ext() const {
	STRPOS pos = mName.find_last_of( "." );
	if ( pos == STRNPOS ) {
		return "";
	}
	else {
		return mName.substr( pos );
	}
}

//---------------------------------------------------------------------------
// Get directory name without trailing separator. If there is no directory,
// returns the empty string.
//---------------------------------------------------------------------------

string Filename :: Dir() const {
	STRPOS pos = mName.find_last_of( "/" );
	if ( pos == STRNPOS ) {
		return "";
	}
	else {
		return mName.substr( 0, pos );
	}
}

//---------------------------------------------------------------------------
// Get drive. For Windows, returns drive name and colon. For UN*X, returns
// empty string.
//---------------------------------------------------------------------------

string Filename :: Drive() const {
	STRPOS pos = mName.find_first_of( ":" );
	if ( pos == STRNPOS ) {
		return "";
	}
	else {
		return mName.substr( 0, pos + 1 );
	}
}

//---------------------------------------------------------------------------
// Read into buffer, returning number of chars read. File will be in bad
// state after the last read.
//---------------------------------------------------------------------------

unsigned int FileRead( std::istream & is, std::vector <char> & buff ) {
	is.read( &buff[0], buff.size() );
	return is.gcount();
}
//---------------------------------------------------------------------------
// Read whole file into string. The file must be opwned in binary read mode.
// Doesn't close the file.
//---------------------------------------------------------------------------

void FileRead( std::ifstream & ifs, string & s ) {
	const unsigned int BUFSIZE = 64 * 1024;	// reasoable sized buffer
	std::vector <char> buffer( BUFSIZE );

	while( unsigned int n = FileRead( ifs, buffer ) ) {
		s.append( &buffer[0], n );
	}
}

//---------------------------------------------------------------------------
// Read all of named file into buffer, hanfling opening and closing of file.
// Throws execeptions on error.
//---------------------------------------------------------------------------

void FileRead( const string & fname, string & s  )  {
	std::ifstream ifs( fname.c_str(), std::ios::in | std::ios::binary );
	if ( ! ifs.is_open() ) {
		ATHROW( "Cannot open " << SQuote( fname ) << " for reading" );
	}
	FileRead( ifs, s );
	ifs.close();
}


//---------------------------------------------------------------------------
// Dores named file (not directory) exist. Doing this is OS specific.
//---------------------------------------------------------------------------

bool FileExists( const string & fname ) {
	Filename fn( fname );
	string dirname = fn.Dir();
	if ( dirname == "" ) {
		dirname = ".";
	}
	DirEnt dir( dirname );
	string fb = fn.Base();
	for ( unsigned int i = 0; i < dir.Size(); i++ ) {
		if ( dir.Filename(i) == fb ) {
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------------
// As above, but check for directory not file.
//---------------------------------------------------------------------------

bool DirExists( const std::string & dirname ) {
	return DirEnt::Exists( dirname );
}


//---------------------------------------------------------------------------

}	// end namespace

//----------------------------------------------------------------------------
// Unit tests
//----------------------------------------------------------------------------

#ifdef ALIB_TEST

#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_file" );

DEFTEST( DirEntTest ) {
	DirEnt d( "." );
	FAILIF( d.Size() == 0 );
	/*
	for ( unsigned int i = 0; i < d.Size(); i++ ) {
		cout << d.Filename( i ) << endl;
	}
	*/
}

DEFTEST( FileTest ) {
	FAILIF( ! FileExists( "Makefile" ) );
	FAILIF( FileExists( "not_there" ) );
	FAILIF( ! DirExists( "src" ) );
	FAILIF( DirExists( "not_there" ) );
	FAILIF( ! DirExists( "/" ) );
}

#endif

// end
