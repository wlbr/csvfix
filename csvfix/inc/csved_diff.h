//---------------------------------------------------------------------------
// csved_diff.h
//
// diff two csv files
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_DIFF_H
#define INC_CSVED_DIFF_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

class DiffCommand : public Command {

	friend class Differ;

	public:

		DiffCommand( const std::string & name,
					const std::string & desc );

		int Execute( ALib::CommandLine & cmd );


	private:

		void ProcessFlags( ALib::CommandLine & cmd );

		FieldList mFields;
		bool mReport, mTrim, mIgnoreCase;
};

}

#endif

