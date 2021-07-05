//---------------------------------------------------------------------------
// csved_dsv.h
//
// delimitter separated values (DSV) read/write
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_DSV_H
#define INC_CSVED_DSV_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {


//---------------------------------------------------------------------------
// Base class for delimiter separated vars read and write
//----------------------------------------------------------------------------


class DSVBase : public Command {

	public:

		DSVBase( const std::string & name,
				 const std::string & desc,
				 const std::string & help );

	public:

		char Delim() const;
		void ReadFlags( const ALib::CommandLine & cl );
		void BuildCSVRow( const CSVRow & in, CSVRow & out ) const;

	private:

		FieldList mFields;
		char mDelim;
};

//---------------------------------------------------------------------------
// Read delimitter separated variables
//----------------------------------------------------------------------------


class DSVReadCommand : public DSVBase {

	public:

		DSVReadCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		std::string Unquote( const std::string & s ) const;

		void ParseDSV( const std::string & line,
							CSVRow & row );
		bool mIsCSV, mCollapseSep;

};

//------------------------------------------------------------------------
// Write delimitter separated variables
//----------------------------------------------------------------------------

class DSVWriteCommand : public DSVBase {

	public:

		DSVWriteCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		std::string MakeDSV( const CSVRow & row );
		std::string MakeField( const std::string & val );

};


}	// namespace

#endif

