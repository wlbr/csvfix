//---------------------------------------------------------------------------
// csved_money.h
//
// do money formatting onn CSV fields
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_MAP_H
#define INC_CSVED_MAP_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class MoneyCommand : public Command {

	public:

		MoneyCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( ALib::CommandLine & cmd );
		std::string FormatValue( const std::string & v ) const;

		FieldList mFields;
		char mDecimalPoint, mThouSep;
		std::string mSymbol, mPlus, mMinus;
		bool mReplace;
		int mWidth;
		bool mCents;
};



//------------------------------------------------------------------------

}	// end namespace

#endif

