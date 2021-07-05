//---------------------------------------------------------------------------
// csved_main.cpp
//
// Main for the CSVfix stream editor
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "csved_cli.h"
#include "csved_except.h"

using namespace CSVED;

//---------------------------------------------------------------------------
// For Windows builds we need to do our own globbing. This is mostly handled
// for us by alib, but we may need to explicitly turn off globbing. The
// following  is for MinGW; for MSVC++ you will need something else.
//----------------------------------------------------------------------------

#ifdef WINNT
int _CRT_glob = 0;
#endif

//----------------------------------------------------------------------------

int main( int argc, char * argv[] ) {

	int result = 0;

	try {
		CLIHandler ch( argc, argv );
		result = ch.ExecCommand();
	}
	catch( const Exception & ex ) {
		result = ExceptionHandler::HandleMyError( ex );
	}
	catch( const std::exception & ex ) {
		result = ExceptionHandler::HandleOtherError( ex );
	}
	catch( ... ) {
		result = ExceptionHandler::HandleUnknownError();
	}

	return result;
}

// end

