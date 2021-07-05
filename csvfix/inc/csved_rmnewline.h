//---------------------------------------------------------------------------
// csved_rmnewline.h
//
// remove embedded newlines for CSV input
//
// Copyright (C) 2011 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_RMNEWLINE_H
#define INC_CSVED_RMNEWLINE_H

#include "a_base.h"
#include "csved_command.h"
#include "csved_util.h"

namespace CSVED {


class RemoveNewlineCommand : public Command {

	public:

		RemoveNewlineCommand( const std::string & name,
							const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
		void RemoveNewlines( CSVRow & row );

		std::string mSep;
		FieldList mFields;
		bool mExcludeAfter;
		void ExpandSep(void);
};

//---------------------------------------------------------------------------


}

#endif

