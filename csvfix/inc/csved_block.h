//---------------------------------------------------------------------------
// csved_block.h
//
// perform actions on blocks of CSV records
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_BLOCK_H
#define INC_CSVED_BLOCK_H

#include "a_base.h"
#include "a_expr.h"
#include "csved_command.h"

namespace CSVED {

class BlockCommand : public Command {


	public:

		BlockCommand( const std::string & name,
					const std::string & desc );

		int Execute( ALib::CommandLine & cmd );


	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
		bool AtEndBlock() ;
		bool AtBeginBlock() ;

		enum Action { None, Keep, Remove, Mark };

		ALib::Expression mBeginEx, mEndEx;
		Action mAction;
		bool mExclusive;
		std::string mBlockMark, mNotMark;

};

}   // namespace

#endif

