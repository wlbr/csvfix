//---------------------------------------------------------------------------
// csved_split.h
//
// split single field int multiple fields
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_SPLIT_H
#define INC_CSVED_SPLIT_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------
// Base split commands
//---------------------------------------------------------------------------

class SplitBase : public Command {

	public:

		SplitBase( const std::string & name,
						const std::string & desc,
						const std::string & help );

		unsigned int Field() const {
			return mField;
		}

		bool Keep() const {
			return mKeep;
		}

	protected:

		void GetCommonFlags( ALib::CommandLine & cl );
		void Insert( CSVRow & row, const CSVRow & split );

		unsigned int mField;
		bool mKeep;
};

//---------------------------------------------------------------------------
// Split at fixed positions
//---------------------------------------------------------------------------

class SplitFixed : public SplitBase {

	public:

		SplitFixed( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void CreatePositions( const std::string & ps );
		void AddPosition( const std::string & spos,
							const std::string & slen );
		void Split( CSVRow & row );

		typedef std::vector <std::pair<unsigned int,unsigned int> > PairVec;
		PairVec mPositions;
};

//---------------------------------------------------------------------------
// Split delimitted by specific character(s)
//---------------------------------------------------------------------------

class SplitChar : public SplitBase {

	public:

		SplitChar( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		enum SplitTrans { stNone, stAlpha2Num, stNum2Alpha };

		void Split( CSVRow & row );
		void TransSplit( CSVRow & row );

		std::string mChars;
		SplitTrans mTrans;
};

//------------------------------------------------------------------------

}	// end namespace

#endif

