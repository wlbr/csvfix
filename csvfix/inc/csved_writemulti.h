//---------------------------------------------------------------------------
// csved_writemulti.h
//
// Convert CSV input to multi-line records
//
// Copyright (C) 2014 Neil Butterworth
//---------------------------------------------------------------------------


#ifndef CSVED_WRITEMULTI_H
#define	CSVED_WRITEMULTI_H

#include "a_base.h"
#include "csved_command.h"
#include "csved_types.h"
#include "csved_ioman.h"
#include <vector>
#include <string>

namespace CSVED {


class WriteMultiCommand : public Command {

	public:

		WriteMultiCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

    private:

		bool GetNewMaster( const CSVRow & row, CSVRow & master );
		void SetMasterFields( const CSVRow & row, CSVRow & master );
		CSVRow MakeDetail( const CSVRow & row );
		void ProcessFlags( const ALib::CommandLine & cmd );
		void WriteRecordSeparator( IOManager & io );
		bool mHaveRecSep;
		std::string mRecSep;
		FieldList mMaster, mDetail;
};

} // namespace

#endif

