//---------------------------------------------------------------------------
// csved_truncpad.h
//
// truncation and padding for CSVED
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------


#ifndef INC_CSVED_TRUNCPAD_H
#define INC_CSVED_TRUNCPAD_H

#include "a_str.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------
// Base class handles flag processuing etc.
//----------------------------------------------------------------------------

class TruncPadBase : public Command {

	public:

		TruncPadBase( const std::string & name,
							const std::string & desc,
							const std::string & help );

		int Execute( ALib::CommandLine & cmd );

	protected:

		virtual void ProcessRow( CSVRow & row, unsigned int ncols,
									const ALib::CommaList & cl  ) = 0;
};

//---------------------------------------------------------------------------
// Perform truncation to N leftmost fields
//----------------------------------------------------------------------------

class TruncCommand : public TruncPadBase {

	public:

		TruncCommand( const std::string & name,
							const std::string & desc );

	protected:

		void ProcessRow( CSVRow & row, unsigned int ncols,
								const ALib::CommaList & cl  );
};

//----------------------------------------------------------------------------
// Pad to N fields by adding rightmost ones
//----------------------------------------------------------------------------

class PadCommand : public TruncPadBase {

	public:

		PadCommand( const std::string & name,
							const std::string & desc );

	protected:

		void ProcessRow( CSVRow & row, unsigned int ncols,
								const ALib::CommaList & cl  );
};

//---------------------------------------------------------------------------

}	// end namespace

#endif

