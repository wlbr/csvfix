//---------------------------------------------------------------------------
// csved_erase.h
//
// Erase fields from CSV records using regular expressions.
//
// Copyright (C) 2014 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_ERASE_H
#define INC_CSVED_ERASE_H

#include <vector>

#include "a_base.h"
#include "a_regex.h"

#include "csved_command.h"
#include "csved_types.h"

namespace CSVED {

//---------------------------------------------------------------------------

class EraseCommand : public Command {

	public:

		EraseCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
        CSVRow EraseFields( const CSVRow & row ) const;
        bool EraseField( const std::string & field ) const;

        struct RegexAction {
            ALib::RegEx mRegex;
            bool mMatch;
            RegexAction( const ALib::RegEx & re, bool match )
                : mRegex( re ), mMatch( match ) {}

        };

		FieldList mFields;
		std::vector <RegexAction> mExprs;
		bool mKeep;
};

//------------------------------------------------------------------------

}	// end namespace

#endif

