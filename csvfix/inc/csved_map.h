//---------------------------------------------------------------------------
// csved_map.h
//
// map from one value to another
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_MAP_H
#define INC_CSVED_MAP_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class MapCommand : public Command {

	public:

		MapCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( ALib::CommandLine & cmd );
		void DoMapping();
		void MapValue( std::string & val );
		std::string Expand( const std::string & val );

		bool mIgnoreCase;
		ALib::CommaList mFrom, mTo;
		FieldList mFields;
		CSVRow mRow;

};



//------------------------------------------------------------------------

}	// end namespace

#endif

