//---------------------------------------------------------------------------
// a_exec.cpp
//
// command execution for alib
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include <iostream>
#include "a_exec.h"
#include "a_str.h"
using std::string;

namespace ALib {

//----------------------------------------------------------------------------
// Create with optional command
//----------------------------------------------------------------------------

Executor :: Executor( const string & cmd  )
	: mPipe( 0 ), mCmd( cmd ), mStream( 0 ) {
}

Executor :: ~Executor() {
	ClosePipe();
	delete mStream;
}

//----------------------------------------------------------------------------
// Close pipe if open
//----------------------------------------------------------------------------

void Executor :: ClosePipe() {
	if ( mPipe ) {
		pclose( mPipe );
		mPipe = 0;
	}
}

//----------------------------------------------------------------------------
// Execute this or stored command, returning stream containing stout
//----------------------------------------------------------------------------

std::istream &  Executor:: Exec( const string & cmd  ) {

	if ( cmd != "" ) {
		mCmd = cmd;
	}

	if ( ALib::IsEmpty( mCmd ) ) {
		ATHROW( "No command in ALib::Executor")
	}

	ClosePipe();
	delete mStream;
	mStream = new std::istringstream;

	mPipe = popen( cmd.c_str(), "r" );
	int rv = 1;
	if ( mPipe ) {
		string s;
		const int BUFSIZE = 100;
		char buf[BUFSIZE];
		while( fgets( buf, BUFSIZE, mPipe ) ) {
			s += buf;
		}
		mStream->str( s );
		rv = pclose( mPipe );
		mPipe = 0;
	}

	if ( rv ) {
		mStream->setstate( std::ios::failbit );
	}

	return * mStream;
}

//----------------------------------------------------------------------------

}	// namespace

//----------------------------------------------------------------------------

/*
#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_exec" );


DEFTEST( BadExecTest ) {
	Executor e;
	std::istream & is = e.Exec( "ls -l" );
	FAILIFM( is, "Stream should be bad, but is not" );
}



DEFTEST( DirTest ) {
	Executor e;
	std::istream & is = e.Exec( "ls -l" );
	string line;
	while( std::getline( is, line ) ) {
		std::cout << ">>>>> " << line << std::endl;
	}
}



#endif
*/

// end
