//---------------------------------------------------------------------------
// csved_exec.h
//
// execute external command
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_EXEC_H
#define INC_CSVED_EXEC_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class ExecCommand : public Command {

	public:

		ExecCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		std::string MakeCmd( const CSVRow & row );
		std::string MakeParam( const CSVRow & row ,unsigned int  & pos );
		std::string mCmdLine;
};

//------------------------------------------------------------------------

}	// end namespace

#endif

