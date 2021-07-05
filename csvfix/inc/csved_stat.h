//---------------------------------------------------------------------------
// csved_stat.h
//
// produce record/field stats for CSV files
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_STAT_H
#define INC_CSVED_STAT_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

class StatCommand : public Command {


	public:

		StatCommand( const std::string & name,
					const std::string & desc );

		int Execute( ALib::CommandLine & cmd );


	private:

		void OutputStats( IOManager & io, const std::string & fname,
							int lines, int maxf, int minf );
};

}

#endif

