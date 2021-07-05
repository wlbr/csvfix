//---------------------------------------------------------------------------
// csved_echo.h
//
// echo input
// this is the simplest possible command & can be used as a template
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSV_ECHO_H
#define INC_CSV_ECHO_H

#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class EchoCommand : public Command {

	public:

		EchoCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

};

//---------------------------------------------------------------------------

}	// end namespace

#endif

