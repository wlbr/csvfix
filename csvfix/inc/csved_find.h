//---------------------------------------------------------------------------
// csved_find.h
//
// regex searching for csvfix
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_FIND_H
#define INC_CSVED_FIND_H

#include "a_base.h"
#include "a_regex.h"
#include "a_expr.h"

#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class FindCommand : public Command {

	public:

		FindCommand( const std::string & name,
						const std::string & desc );
		~FindCommand();

		int Execute( ALib::CommandLine & cmd );

		std::string Help() const;

	private:

		void Clear();
		void CreateRegExes( const ALib::CommandLine & cmd );
		void CreateRanges( const ALib::CommandLine & cmd );
		void CreateLengths( const ALib::CommandLine & cmd );
		void CreateFieldCounts( const ALib::CommandLine & cmd );

		bool MatchRow( CSVRow & row );
		bool TryAllRegExes( const std::string & s );
		bool TryAllRanges( const std::string & s );
		bool TryAllLengths( const std::string & s );
		bool HaveRegex() const;

		std::vector <ALib::RegEx *> mExprs;
		std::vector <unsigned int> mColIndex;

		typedef std::pair<std::string,std::string> RangeData;
		struct Range {
			RangeData mRange;
			bool mIsNum;
			Range( const RangeData & r, bool isnum )
				: mRange( r ), mIsNum( isnum ) {}
		};

		std::vector <Range> mRanges;


		typedef std::pair< int, int> LenRange;
		std::vector <LenRange> mLengths;

		bool mRemove;
		bool mCountOnly;

		int mMinFields, mMaxFields;

		ALib::Expression mEvalExpr;
};


//------------------------------------------------------------------------

}	// end namespace

#endif

