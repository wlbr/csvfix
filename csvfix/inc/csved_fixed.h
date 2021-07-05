//---------------------------------------------------------------------------
// csved_fixed.h
//
// read/write fixed format files
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_FIXED_H
#define INC_CSVED_FIXED_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {


//---------------------------------------------------------------------------
// Base for the fixed read/write commands supports field specifications.
//----------------------------------------------------------------------------

class FixedCommand : public Command {

	public:

		FixedCommand( const std::string & name,
							const std::string & desc,
							const std::string & help );

	protected:

		void BuildFields( const ALib::CommandLine & cl );
		typedef std::pair<unsigned int ,unsigned int> FieldSpec;
		std::vector <FieldSpec> mFields;

};

//----------------------------------------------------------------------------
// Read fixed fields described by field specs and convert to CSV.
//----------------------------------------------------------------------------

class ReadFixedCommand : public FixedCommand {

	public:

		ReadFixedCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );


	private:

		void MakeRow( const std::string & line, CSVRow & row );

		bool mTrim;
};

//---------------------------------------------------------------------------
// Write CSV as fixed format using field specs to describe output positions.
//----------------------------------------------------------------------------

class WriteFixedCommand : public FixedCommand {

	public:

		WriteFixedCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		std::string MakeFixedOutput( const CSVRow & row );

};

//------------------------------------------------------------------------

}	// end namespace

#endif

