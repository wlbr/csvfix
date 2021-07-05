//---------------------------------------------------------------------------
// csved_readmulti.h
//
// multi line to csv conversion
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_READMULTI_H
#define INC_CSVED_READMULTI_H

#include "csved_command.h"

namespace CSVED {

//----------------------------------------------------------------------------

class ReadMultiCommand : public Command {

	public:

		ReadMultiCommand( const std::string & name,
							const std::string & desc );

		int Execute( ALib::CommandLine & cmd );
		void ProcessFlags( ALib::CommandLine & cmd );

	private:

		std::string mSep;
		unsigned int mNumLines;

};

//----------------------------------------------------------------------------

} // namespace



#endif

