//---------------------------------------------------------------------------
// csved_merge.h
//
// merge multiple sorted CSV files
//
// Copyright (C) 2011 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_FMERGE_H
#define INC_CSVED_FMERGE_H

#include "a_base.h"
#include "csved_command.h"
#include <stack>
#include <vector>
#include <memory>

namespace CSVED {

class RowGetter {

	public:

		RowGetter( ALib::CSVStreamParser * p );
		~RowGetter();

		bool Get( CSVRow & row );
		void ClearLatch();

	private:

		ALib::CSVStreamParser * mParser;
		bool mDone, mHave;
		CSVRow mLatch;
};

class MinFinder {

	public:

		MinFinder( IOManager & io, const FieldList & fields );
		~MinFinder();

		bool FindMin( CSVRow & row );

	private:

		std::vector <RowGetter *> mGetters;
		const FieldList & mFields;

};

class FMergeCommand : public Command {

	public:

		FMergeCommand( const std::string & name,
					const std::string & desc );

		int Execute( ALib::CommandLine & cmd );


	private:

		void ProcessFlags( ALib::CommandLine & cmd );

		FieldList mFields;


};

}

#endif

