//---------------------------------------------------------------------------
// csved_headtail.h
//
// head and tail commands for csvfix
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_HEADTAIL_H
#define INC_CSVED_HEADTAIL_H

#include <list>
#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class HeadCommand : public Command {

	public:

		HeadCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		unsigned int mRecords;

};

class TailCommand : public Command {

	public:

		TailCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		unsigned int mRecords;
		std::list <CSVRow> mLastRows;
};

//------------------------------------------------------------------------

}	// end namespace

#endif

