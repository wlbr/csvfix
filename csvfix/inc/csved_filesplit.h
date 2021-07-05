//---------------------------------------------------------------------------
// csved_filesplit.h
//
// split CSV stream into multiple files depending on fields
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_FILESPLIT_H
#define INC_CSVED_FILESPLIT_H

#include "a_base.h"
#include "csved_command.h"
#include "a_dict.h"
#include <fstream>

namespace CSVED {

//----------------------------------------------------------------------------


class FileSplitCommand : public Command {

	public:

		FileSplitCommand( const std::string & name,
							const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( ALib::CommandLine & cmd );
		std::string NewFileName( const std::string &  key );
		void WriteRow( IOManager & io, const CSVRow & row );
		std::string MakeKey( const CSVRow & row );

		std::string mDir, mFilePrefix, mFileExt, mCurrentFile;
		ALib::Dictionary <std::string> mDict;
		std::vector <unsigned int> mColIndex;
		unsigned int mFileNo;
		std::ofstream mOutFile;
		bool mUseFieldNames;
};

//----------------------------------------------------------------------------

} // namespace


#endif


