//---------------------------------------------------------------------------
// csved_number.h
//
// convert fields possibly containing strange thousands/decimal point
// separators to numeric values
//
// Copyright (C) 2013 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_NUMBER_H
#define INC_CSVED_NUMBER_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

class NumberCommand : public Command {


	public:

		NumberCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );


	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
		void Convert( CSVRow & row );
		std::string ConvertField( const std::string & val, char ts, char dp );

		std::vector <unsigned int> mFields;
		std::string mFormat, mErrStr;
		bool mErrExit, mHasErrStr;

};

}   // namespace

#endif

