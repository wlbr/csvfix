//---------------------------------------------------------------------------
// a_exec.h
//
// command execution for alib
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_EXEC_H
#define INC_A_EXEC_H

#include "a_base.h"
#include <stdio.h>
#include <sstream>

namespace ALib {

//----------------------------------------------------------------------------

class Executor {

	public:

		Executor( const std::string & cmd = "" );
		~Executor();

		std::istream & Exec( const std::string & cmd = "" );

	private:

		void ClosePipe();

		FILE * mPipe;
		std::string mCmd;
		std::istringstream * mStream;
};

//------------------------------------------------------------------------

}  // namespace


#endif

