//---------------------------------------------------------------------------
// csved_timestamp.h
//
// add timestamp field to CSV input
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_TIMESTAMP_H
#define INC_CSVED_TIMESTAMP_H

#include "a_base.h"
#include "a_expr.h"
#include "csved_command.h"
#include <ctime>

namespace CSVED {


//----------------------------------------------------------------------------
// Timestamp command
//----------------------------------------------------------------------------

class TimestampCommand : public Command {

	public:

		TimestampCommand( const std::string & name,
					const std::string & desc );

		int Execute( ALib::CommandLine & cmd );


	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
		std::string FormatStamp( std::time_t t ) const;

		bool mRealTime, mShowDate, mShowTime, mNumericStamp;

};

//----------------------------------------------------------------------------

} // namespace

#endif
