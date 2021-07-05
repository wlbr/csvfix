//---------------------------------------------------------------------------
// csved_flatten.h
//
// flatten and unflatten for csvfix
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_FLATTEN_H
#define INC_CSVED_FLATTEN_H

#include "a_base.h"
#include "a_expr.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class FlattenCommand : public Command {

	public:

		FlattenCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
		std::string MakeKey( const CSVRow & row ) const;
		void NewKey( const CSVRow & row );
		void AddData( const CSVRow & row );
		int MDFlatten( ALib::CommandLine & cmd );

		std::string mKey;
		FieldList mKeyFields, mDataFields;
		CSVRow mData;
		bool mKeepKey;
		std::string mMasterExpr;

};

//------------------------------------------------------------------------

class UnflattenCommand : public Command {

	public:

		UnflattenCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
		CSVRow GetKey( const CSVRow & row ) const;
		void WriteRows( IOManager & io, const CSVRow & key,
							const CSVRow & row );
		unsigned int mNumDataFields;
		FieldList mKeyFields;

};

}	// end namespace

#endif

