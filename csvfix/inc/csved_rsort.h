//---------------------------------------------------------------------------
// csved_rsort.h
//
// Do in-row sorting of CSV fields
//
// Copyright (C) 2014 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_RSORT_H
#define INC_CSVED_RSORT_H

#include "a_base.h"
#include "csved_command.h"
#include <vector>
#include <string>

namespace CSVED {



//---------------------------------------------------------------------------

class RowSortCommand : public Command {

	public:

		RowSortCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
        void SortRow( CSVRow & row );
        std::vector <std::string> GetSortFields( const CSVRow & row ) ;
        void PutSortFields(  CSVRow & row, const std::vector <std::string> & sf ) const;

		FieldList mFields;
		unsigned int mStartPos;
		bool mSortAscending, mSortLex;

};

//------------------------------------------------------------------------

}	// end namespace

#endif

