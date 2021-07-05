//---------------------------------------------------------------------------
// csved_put.h
//
// put fixed text or environment var into CSV
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_PUT_H
#define INC_CSVED_PUT_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class PutCommand : public Command {

	public:

		PutCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( ALib::CommandLine & cmd );

		int mPos;
		std::string mValue;

};

//------------------------------------------------------------------------

}	// namespace

#endif

