//---------------------------------------------------------------------------
// csved_check.h
//
// check CSV records actually are CSV
//
// Copyright (C) 2011 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_CHECK_H
#define INC_CSVED_CHECK_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class CheckCommand : public Command {

	public:

		CheckCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( const ALib::CommandLine & cmd );

		bool mQuiet, mEmbedNLOk, mVerbose;
		char mSep;

};

//------------------------------------------------------------------------

}	// end namespace

#endif

