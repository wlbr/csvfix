//---------------------------------------------------------------------------
// csved_except.cpp
//
// CSVfix exception stuff - none of these do anything particularly smart
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include <iostream>
#include "csved_except.h"

namespace CSVED {

//---------------------------------------------------------------------------
// Handle all CSVfix specific exceptions that make their way into main()
//---------------------------------------------------------------------------

int ExceptionHandler :: HandleMyError( const CSVED::Exception & ex ) {
	std::cerr << "ERROR: " << ex.what() << std::endl;
	return 1;
}

//---------------------------------------------------------------------------
// Handle anything derived from std::exception that makes it to main()
//---------------------------------------------------------------------------

int ExceptionHandler :: HandleOtherError( const std::exception & ex ) {
	std::cerr << "ERROR: " << ex.what() << std::endl;
	return 1;
}

//---------------------------------------------------------------------------
// Handle anything else
//---------------------------------------------------------------------------

int ExceptionHandler :: HandleUnknownError() {
	std::cerr << "UNKNOWN EXCEPTION" << std::endl;
	return 1;
}

//---------------------------------------------------------------------------

}	// namespace

// end

