//---------------------------------------------------------------------------
// csved_fileinfo.h
//
// add file ibformation to csv rows
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_FILEINFO_H
#define INC_CSVED_FILEINFO_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class FileInfoCommand : public Command {

	public:

		FileInfoCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		bool mBasename;
		bool mTwoCols;
};

//------------------------------------------------------------------------

}	// end namespace

#endif

