//---------------------------------------------------------------------------
// csved_shuffle.h
//
// random shuffle CSV rows and fields
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_SHUFFLE_H
#define INC_CSVED_SHUFFLE_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class ShuffleCommand : public Command {

	public:

		ShuffleCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
		void Shuffle( IOManager & io );
		void ShuffleFields( CSVRow & row );
		std::vector <CSVRow> mRows;
		unsigned int mCount;
		int mSeed;
		FieldList mFields;
};

//------------------------------------------------------------------------

}	// end namespace

#endif

