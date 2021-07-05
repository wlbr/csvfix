//---------------------------------------------------------------------------
// csved_atable.h
//
// ascii tables for csvfix
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_ATABLE_H
#define INC_CSVED_ATABLE_H

#include "csved_command.h"
#include <iosfwd>

namespace CSVED {

//----------------------------------------------------------------------------

class AsciiTableCommand : public Command {

	public:

		AsciiTableCommand( const std::string & name,
							const std::string & desc );

		int Execute( ALib::CommandLine & cmd );
		void ProcessFlags( ALib::CommandLine & cmd );

	private:

		void AddRow( const CSVRow & row );
		void OutputTable( std::ostream & os );
		void OutputRow( std::ostream & os, const CSVRow & row  );
		void OutputHeadings( std::ostream & os, const CSVRow & row  );
		std::string MakeSep() const;

		std::vector <CSVRow> mRows;
		std::vector <unsigned int> mWidths;
		ALib::CommaList mHeadings;
		FieldList mRightAlign;
		bool mUseLineSep;
};

//----------------------------------------------------------------------------

} // namespace

#endif

