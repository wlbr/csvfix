//---------------------------------------------------------------------------
// a_sort.h
//
// array sorting on multiple columns for alib
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_SORT_H
#define INC_A_SORT_H

#include "a_base.h"

namespace ALib {

//---------------------------------------------------------------------------
// Field specifies which columns to sort on etc.
//---------------------------------------------------------------------------

class SortField {

	friend class Sorter;

	public:

		enum CmpType { ctAlpha, ctNumeric, ctNoCase };
		enum Direction { dirAsc, dirDesc };

		SortField( unsigned int index,
						Direction d = dirAsc,
						CmpType ct = ctAlpha);

		bool Less( const std::string & s1,
					const std::string & s2 ) const;

	private:

		unsigned int mIndex;
		Direction mDir;
		CmpType mCmpType;

};

//---------------------------------------------------------------------------
// Sorter performs sort
//---------------------------------------------------------------------------

class Sorter {

	public:

		typedef std::vector <std::string> RowType;
		typedef std::vector <RowType> ArrayType;

		Sorter();
		virtual ~Sorter();

		void AddField( const SortField & f );
		void Reset();

		void Sort( ArrayType & a );

		bool operator()( const RowType & r1, const RowType & r2 );

	private:

		std::vector <SortField> mFields;

};

//---------------------------------------------------------------------------

}	// end namespace


#endif

