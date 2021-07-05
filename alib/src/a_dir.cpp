//----------------------------------------------------------------------------
// a_dir.cpp
//
// portable directory listing
//
// Copyright (C) 2009 Neil Butterworth
//----------------------------------------------------------------------------

#include "a_dir.h"
#include "a_collect.h"
#include "a_str.h"
#include "a_win.h"
using std::string;

namespace ALib {

//----------------------------------------------------------------------------
// Empty ctor and virtual dtor for base are common
//----------------------------------------------------------------------------

DirListEntry :: DirListEntry() {
}

DirListEntry :: ~DirListEntry() {
}

//----------------------------------------------------------------------------

#ifdef ALIB_WINAPI

//----------------------------------------------------------------------------
// Windows implementation of DirList entry
//----------------------------------------------------------------------------

class WinDirEntry : public DirListEntry {

	public:

		WinDirEntry( const std::string & f, bool isdir )
			: mName( f ), mIsDir( isdir ) {}

		string Name() const {
			return mName;
		}

		bool IsDir() const {
			return mIsDir;
		}

		bool IsFile() const {
			return ! IsDir();
		}

	private:

		string mName;
		bool mIsDir;

};

//----------------------------------------------------------------------------
// Create directory list, possinly populating it
//----------------------------------------------------------------------------

DirList :: DirList( const string & fspec ) {
	if ( fspec != "" ) {
		Populate( fspec );
	}
}

//----------------------------------------------------------------------------
// Clear listing
//----------------------------------------------------------------------------

DirList :: ~DirList() {
	Clear();
}

//----------------------------------------------------------------------------
// Static functions to test if a directory or file exists
//----------------------------------------------------------------------------

static DWORD FileAttr( const string & f ) {
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile( f.c_str(), & fd );
	if ( h == INVALID_HANDLE_VALUE ) {
		return 0;
	}
	else {
		FindClose( h );
		return fd.dwFileAttributes;
	}
}

bool DirList :: DirExists( const string & dir ) {
	return FileAttr( dir ) & FILE_ATTRIBUTE_DIRECTORY;
}


bool DirList :: FileExists( const string & file ) {
	DWORD a = FileAttr( file );
	return a == 0 ? 0 : ! ( a & FILE_ATTRIBUTE_DIRECTORY );
}

//----------------------------------------------------------------------------
// Populate DirList structure using wildcarded file spec, clearing first
//----------------------------------------------------------------------------

unsigned int DirList :: Populate( const string & fspec ) {
	Clear();
	return Add( fspec );
}

//----------------------------------------------------------------------------
// Add iles matching file spec to listing, returning number added. Each
// filename is prepended with its path, if one was supplied in file spec.
//----------------------------------------------------------------------------

unsigned int DirList :: Add( const string & fspec ) {

    // save path, if any
	STRPOS lastslash = fspec.find_last_of( "\\/");
	string path = lastslash == STRNPOS ? "" : fspec.substr( 0, lastslash + 1 );

	unsigned int count = 0;
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile( fspec.c_str(), & fd );
	if ( h == INVALID_HANDLE_VALUE ) {
		return count;
	}

	do {
		mEntries.push_back(
			new WinDirEntry(
					path + fd.cFileName,	// add path back on
					fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
			)
		);
		count++;
	} while( FindNextFile( h, & fd ) );

	FindClose( h );

	return count;
}

//----------------------------------------------------------------------------
// Add all files matching specifications sepcified by fspec using sep
// as a separator.
//----------------------------------------------------------------------------

unsigned int DirList :: Add( const std::string & fspec, char sep ) {
	std::vector <string> s;
	unsigned int scount = Count();
	if ( ALib::Split( fspec, sep, s ) == 0 ) {
		ATHROW( "Empty DirList file spec");
	}
	for ( unsigned int  i = 0; i < s.size(); i++ ) {
		Add( s[i] );
	}

	return Count() - scount;
}

//----------------------------------------------------------------------------
// Dispose of dir entries & clear list
//----------------------------------------------------------------------------

void DirList :: Clear() {
	FreeClear( mEntries );
}

//----------------------------------------------------------------------------
// How many entries?
//----------------------------------------------------------------------------

unsigned int DirList :: Count() const {
	return mEntries.size();
}

//----------------------------------------------------------------------------
// Get entry at index
//----------------------------------------------------------------------------

const DirListEntry * DirList :: At( unsigned int i ) const {
	if ( i >= Count() ) {
		ATHROW( "Invalid DirList index: " << i );
	}
	return mEntries[i];
}


//----------------------------------------------------------------------------
// We only do Win32 at present
//----------------------------------------------------------------------------

#else
#error "Windows only at present"
#endif

//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------
// Tests
//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_dir" );


DEFTEST( SimpleTest ) {
	DirList dir( "*.*" );
	FAILEQ( dir.Count(), 0 );
/*
	for ( unsigned int i = 0; i < dir.Count(); i++ ) {
		std::cerr << dir.At(i)->Name() << "\n";
	}
*/
}

DEFTEST( SrcTest ) {
	DirList dir( "src" );
	FAILNE( dir.Count(), 1 );
	FAILNE( dir.At(0)->Name(), "src" );
	FAILNE( dir.At(0)->IsDir(), true );
	FAILNE( dir.At(0)->IsFile(), false );
}


DEFTEST( ExistsTest ) {
	bool ok = DirList::DirExists( "src" );		// existing directory
	FAILNE( ok, true );
	ok = DirList::DirExists( "bad_bad_bad" );	// no such thing
	FAILNE( ok, false );
	ok = DirList::DirExists( "Makefile" );		// existing file
	FAILNE( ok, false );
	ok = DirList::FileExists( "src" );			// existing directory
	FAILNE( ok, false );
	ok = DirList::FileExists( "bad_bad_bad" );	// no such thing
	FAILNE( ok, false );
	ok = DirList::FileExists( "Makefile" );		// existing file
	FAILNE( ok, true );
}

DEFTEST( WildCardTest ) {
	DirList dir( "src/*.cpp" );
	FAILIF( dir.Count() < 10 );
//	for ( unsigned int i = 0; i < dir.Count(); i++ ) {
//		std::cerr << dir.At(i)->Name() << "\n";
//	}
}

#endif

// end
