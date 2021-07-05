//---------------------------------------------------------------------------
// csved_edit.h
//
// edit fields for csved
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_EDIT_H
#define INC_CSVED_EDIT_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class EditCommand : public Command {

	public:

		EditCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void EditRow( CSVRow & row );
		void EditField( std::string & f);
		void AddSubCmd( const std::string & s );

		struct EditSubCmd {
			char mCmd;
			std::string mFrom, mTo;
			std::string mOpts;
		
			EditSubCmd( char cmd, 
							const std::string & from,
							const std::string & to, 
							const std::string & opts )
				: mCmd( cmd ), mFrom( from ), mTo( to ), mOpts( opts ) {
			}
		};

		std::vector <EditSubCmd> mSubCmds;
		std::vector <unsigned int> mCols;

};

//------------------------------------------------------------------------

}	// end namespace

#endif

