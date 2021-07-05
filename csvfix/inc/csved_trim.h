//---------------------------------------------------------------------------
// csved_trim.h
//
// trim leading/trailing spaces
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_TRIM_H
#define INC_CSVED_TRIM_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class TrimCommand : public Command {

	public:

		TrimCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void Trim( CSVRow & row );
		void GetWidths( const std::string & ws );
		void Chop( CSVRow & row, unsigned int  i );

		FieldList mFields;
		bool mTrimLead, mTrimTrail;
		std::vector <int> mWidths;


};


//------------------------------------------------------------------------

}	// end namespace

#endif

