//---------------------------------------------------------------------------
// csved_merge.h
//
// merge csv column data
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_MERGE_H
#define INC_CSVED_MERGE_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class MergeCommand : public Command {

	public:

		MergeCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
		void DoMerge( CSVRow & row );
		void BuildNewRow( CSVRow & row, const std::string & merged );

		std::vector <unsigned int> mCols;
		unsigned int mPos;
		std::string mSep;
		bool mKeep;
};

//------------------------------------------------------------------------

}	// end namespace

#endif

