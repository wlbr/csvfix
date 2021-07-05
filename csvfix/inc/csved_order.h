//---------------------------------------------------------------------------
// csved_order.h
//
// column reordering for CSVfix
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_ORDER_H
#define INC_CSVED_ORDER_H

#include "a_str.h"
#include "csved_command.h"
#include "csved_ioman.h"

namespace CSVED {

//----------------------------------------------------------------------------
// Order command needs to monitor changes in named (as opposed to numbered)
// field order when a new file is opened, as the new file may have a different
// field name header, so it is derived from IOWatcher as well as from the
// usual Command stuff.
//----------------------------------------------------------------------------

class OrderCommand : public Command, public IOWatcher {

	public:

		OrderCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

		void OnNewCSVStream( const std::string & filename,
							 const ALib::CSVStreamParser * p );


	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
		void MakeOrder( const ALib::CommandLine & cmd );
		void Reorder( CSVRow & row );
		void ExcludeFields( CSVRow & row );

		FieldList mOrder;
		ALib::CommaList mOrderNames;
		bool mRevOrder;
		bool mExclude;
		bool mNoCreate;
};

//---------------------------------------------------------------------------

}	// end namespace


#endif

