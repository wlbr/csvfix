//---------------------------------------------------------------------------
// csved_squash.h
//
// squash duplicate rows to single row
//
// Copyright (C) 2014 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_SQUASH_H
#define INC_CSVED_SQUASH_H

#include "a_base.h"
#include "csved_command.h"
#include <map>
#include <vector>
#include <string>
#include <memory>

namespace CSVED {


class SquashValues {

    public:

        SquashValues() {}
        virtual ~SquashValues() {}
        virtual void Add( unsigned int idx, const std::string & val ) = 0;
        virtual void AppendTo( CSVRow & row ) = 0;
};

//---------------------------------------------------------------------------

class SquashCommand : public Command {

	public:

		SquashCommand( const std::string & name,
							const std::string & desc );

		int Execute( ALib::CommandLine & cmd );


	private:

        void ProcessFlags( const ALib::CommandLine & cmd );
        std::string MakeKey( const CSVRow & row ) const;
        void Accumulate( std::shared_ptr<SquashValues>& vp, const CSVRow & row  );
        void Key2CSV( const std::string & key, CSVRow & row);

        FieldList mKeyFields, mValFields;
        std::map <std::string, std::shared_ptr<SquashValues>> mKeyValues;
        std::string mNotNumVal;
        bool mRealNums;

};

//------------------------------------------------------------------------

}	// end namespace

#endif

