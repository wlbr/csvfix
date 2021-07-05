//---------------------------------------------------------------------------
// csved_sort.h
//
// sort command for CSVED
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_SORT_H
#define INC_CSVED_SORT_H

#include "a_base.h"
#include "a_sort.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class SortCommand : public Command {

	public:

		SortCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void BuildFieldSpecs( const ALib::CommandLine & cmd );
		void Sort( std::vector <CSVRow> & rows );

		std::vector <ALib::SortField> mFields;

};

//------------------------------------------------------------------------

}	// end namespace

#endif

