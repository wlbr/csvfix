//---------------------------------------------------------------------------
// csved_inter.h
//
// interleave fields from two CSv sources
//
// Copyright (C) 2010 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_INTER_H
#define INC_CSVED_INTER_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class InterCommand : public Command {

	public:

		InterCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void ProcessFlags( const ALib::CommandLine & cmd ) ;

		struct FieldSpec {
			unsigned int mSrc;
			unsigned int mField;
			FieldSpec( unsigned int src, unsigned int fi )
				: mSrc( src ), mField( fi ) {}
		};

		CSVRow Interleave( const CSVRow & r1 , const CSVRow & r2 ) const;
		std::string GetField( const FieldSpec & f,  const CSVRow & r1 ,
											const CSVRow & r2 ) const;

		FieldSpec MakeField( const std::string &  f ) const;
		std::vector <FieldSpec> mFields;
};

//------------------------------------------------------------------------

}	// end namespace

#endif

