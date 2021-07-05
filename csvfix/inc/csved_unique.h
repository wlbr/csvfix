//---------------------------------------------------------------------------
// csved_unique.h
//
// remove duplicate rows
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_UNIQUE_H
#define INC_CSVED_UNIQUE_H

#include "a_base.h"
#include "csved_command.h"
#include <map>

namespace CSVED {

//---------------------------------------------------------------------------

class UniqueCommand : public Command {

	public:

		UniqueCommand( const std::string & name,
							const std::string & desc );

		int Execute( ALib::CommandLine & cmd );


	private:

		struct RowInfo {
			CSVRow mFirst;
			unsigned int mCount;

			RowInfo( const CSVRow & row ) 
				: mFirst( row ), mCount( 1 ) {}
		};

		
		std::string MakeKey( const CSVRow & row ) const;

		void FilterUnique( IOManager & io, const CSVRow & row );
		void FilterDupes( IOManager & io, const CSVRow & row );

		bool mShowDupes;

		typedef std::map <std::string,RowInfo> MapType;
		MapType mMap;
		std::vector <unsigned int> mCols;

};

//------------------------------------------------------------------------

}	// end namespace

#endif

