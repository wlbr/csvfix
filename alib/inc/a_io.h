//----------------------------------------------------------------------------
// a_io.h
//
// various I/O stuff for alib
//
// Copyright (C) 20010 Neil Butterworth
//----------------------------------------------------------------------------

#ifndef INC_ALIB_IO_H
#define INC_ALIB_IO_H

#include "a_str.h"
#include <iostream>
#include <fstream>
#include <sstream>

//----------------------------------------------------------------------------
// Redirector for streams
//----------------------------------------------------------------------------

template <typename FROMSTREAM, typename TOSTREAM>
class Redirector {

	public:

		Redirector( FROMSTREAM & os, TOSTREAM & fs )
			: mOs( os ), mOsBuffer(0) {
			mOsBuffer = os.rdbuf();
			os.rdbuf( fs.rdbuf() );
		}

		void End() {
			if ( mOsBuffer ) {
				mOs.rdbuf( mOsBuffer );
				mOsBuffer = 0;
			}
		}

		virtual ~Redirector() {
			End();
		}

	private:

		FROMSTREAM & mOs;
		std::streambuf * mOsBuffer;
};

//----------------------------------------------------------------------------
// Specialised versions for input & output stream redirectiom
//----------------------------------------------------------------------------

class OutputRedirector : public Redirector  <std::ostream, std::ofstream> {
	public:
		OutputRedirector( std::ostream & os, std::ofstream & fs )
			: Redirector <std::ostream, std::ofstream>( os, fs ) {
		}
};

class InputRedirector : public Redirector  <std::istream,std::ifstream> {
	public:
		InputRedirector( std::istream & os, std::ifstream & fs )
			: Redirector <std::istream, std::ifstream>( os, fs ) {
		}
};

//----------------------------------------------------------------------------
// Convert stringstream to something that supports push_back
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#endif
