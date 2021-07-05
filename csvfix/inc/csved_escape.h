//---------------------------------------------------------------------------
// csved_escape.h
//
// This command was previously called "quote" - now caled "escape"
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_ESCAPE_H
#define INC_CSVED_ESCAPE_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class EscCommand : public Command {

	public:

		EscCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void GetColumns( const ALib::CommandLine & cmd );
		void EscapeRow( CSVRow & row );
		void EscapeValue( CSVRow & row, unsigned int i );

		bool mSqlMode;
		std::string mSpecial, mEsc;
		std::vector <unsigned int> mCols;

};


//------------------------------------------------------------------------

}	// end namespace

#endif

