//---------------------------------------------------------------------------
// a_sort.cpp
//
// array sorting on multiple columns for alib
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_str.h"
#include "a_sort.h"
#include "a_except.h"
#include <algorithm>

namespace ALib {

//---------------------------------------------------------------------------
// Field specifies index in row, sort direction and whether field is
// numeric or string
//---------------------------------------------------------------------------

SortField :: SortField( unsigned int index, Direction d, CmpType ct )
	: mIndex( index ), mDir( d ), mCmpType( ct ) {
}

//---------------------------------------------------------------------------
// Do coparison on string using direction and type.
//---------------------------------------------------------------------------

bool SortField :: Less( const std::string & s1,
						const std::string & s2 ) const {
	if ( mCmpType == ctAlpha ) {
		return mDir == dirAsc ? s1 < s2 : s2 < s1;
	}
	else if ( mCmpType  == ctNumeric ) {
		double d1 = ToReal( s1 );
		double d2 = ToReal( s2 );
		return mDir == dirAsc ? d1 < d2 : d2 < d1;
	}
	else if ( mCmpType == ctNoCase ) {
		int r = ALib::Cmp( s1, s2, IgnoreCase );
		return mDir == dirAsc ? r < 0 : r > 0;
	}
	else {
		ATHROW( "Invalid compare type in ALib::SortField" );
	}
}

//---------------------------------------------------------------------------
// Nothing to do
//---------------------------------------------------------------------------

Sorter :: Sorter() {
	// nothing
}

//---------------------------------------------------------------------------
// Do reset
//---------------------------------------------------------------------------

Sorter :: ~Sorter() {
	Reset();
}

//---------------------------------------------------------------------------
// Add field to sort. Duplicate field indeces are not allowed
//---------------------------------------------------------------------------

void Sorter :: AddField( const SortField & f ) {
	for ( unsigned int i = 0; i < mFields.size(); i++ ) {
		if ( mFields[i].mIndex == f.mIndex ) {
			ATHROW( "Duplicate SortField index " << f.mIndex );
		}
	}
	mFields.push_back( f );
}

//---------------------------------------------------------------------------
// Remove all fields
//---------------------------------------------------------------------------

void Sorter :: Reset() {
	mFields.clear();
}

//---------------------------------------------------------------------------
// Do < comparison on two rows using fields
//---------------------------------------------------------------------------

bool Sorter :: operator()( const RowType & r1, const RowType & r2 ) {
	unsigned int nc = ALib::Min( r1.size(), r2.size() );
	for ( unsigned int i = 0; i < mFields.size(); i++ ) {
		const SortField & f = mFields[i];
		if ( f.mIndex >= nc || r1[f.mIndex] == r2[f.mIndex] ) {
			continue;
		}
		else if ( f.Less( r1[f.mIndex], r2[f.mIndex ] ) ) {
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}



//---------------------------------------------------------------------------
// Sort the array
//---------------------------------------------------------------------------

void Sorter :: Sort( ArrayType & a ) {
	if ( mFields.size() == 0 ) {
		ATHROW( "No fields to sort on specified" );
	}
	std::sort( a.begin(), a.end(), *this );

}

//---------------------------------------------------------------------------

}	// end namespace

//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_sort" );

struct DataRow {
	const char * data[3];
};

// basic stuff
DataRow rows1[] = {
	{ {"one", "1", "A" } },
	{ {"two", "2", "B" } },
	{ {"three", "3", "C" } },
	{ {"four", "4", "D" } } ,
	{ {0, 0, 0 } }
};

// repeating fields
DataRow rows2[] = {
	{ {"one", "1", "A"} },
	{ {"two", "1", "X"} },
	{ {"two", "3", "Y"} },
	{ {"two", "2", "Z"} },
	{ {"three", "3", "C"} },
	{ {"four", "4", "D"} },
	{ {0, 0, 0} }
};

// mixed case
DataRow rows3[] = {
	{ {"One", "1", ""} },
	{ {"two", "2", ""} },
	{ {"TwO", "2", ""} },
	{ {"two", "2", ""} },
	{ {"three", "3", ""} },
	{ {"FOUR", "4", ""} },
	{ {0, 0, 0} }
};

void MakeData( const DataRow * d, Sorter::ArrayType & a ) {
	a.clear();
	int i = 0;
	while( d[i].data[0] ) {
		Sorter::RowType r;
		r.push_back( d[i].data[0] );
		r.push_back( d[i].data[1] );
		r.push_back( d[i].data[2] );
		a.push_back( r );
		i++;
	}
}

void Dump( const Sorter::ArrayType & a ) {
	for ( unsigned int i = 0; i < a.size(); i++ ) {
		Sorter::RowType r = a[i];
		for ( unsigned int j = 0; j < r.size(); j++ ) {
			cout << "[" << r[j] << "]";
		}
		cout << endl;
	}
}

static Sorter::ArrayType ta;

DEFTEST( SimpleTest ) {
	MakeData( rows1, ta );
	Sorter s;
	s.AddField( SortField(0) );
	s.Sort( ta );
	//Dump( ta );
	FAILNE( ta.size(), 4 );
	FAILNE( ta[0][0], "four" );
	FAILNE( ta[1][0], "one" );
	FAILNE( ta[2][0], "three" );
	FAILNE( ta[3][0], "two" );
}

DEFTEST( DescTest ) {
	MakeData( rows1, ta );
	Sorter s;
	s.AddField( SortField(0, SortField::dirDesc ) );
	s.Sort( ta );
	//Dump( ta );
	FAILNE( ta.size(), 4 );
	FAILNE( ta[0][0], "two" );
	FAILNE( ta[1][0], "three" );
	FAILNE( ta[2][0], "one" );
	FAILNE( ta[3][0], "four" );
}


DEFTEST( NoCaseTest ) {
	MakeData( rows3, ta );
	Sorter s;
	s.AddField( SortField(0, SortField::dirAsc, SortField::ctNoCase ) );
	s.Sort( ta );
	//Dump( ta );
	FAILNE( ta.size(), 6 );
	FAILNE( ta[0][1], "4" );
	FAILNE( ta[1][1], "1" );
	FAILNE( ta[2][1], "3" );
	FAILNE( ta[3][1], "2" );
	FAILNE( ta[4][1], "2" );
	FAILNE( ta[5][1], "2" );
}

DEFTEST( TwoFieldsTest ) {
	MakeData( rows2, ta );
	Sorter s;
	s.AddField( SortField(0) );
	s.AddField( SortField(1) );
	s.Sort( ta );
	//Dump( ta );
	FAILNE( ta.size(), 6 );
	FAILNE( ta[3][0], "two" );
	FAILNE( ta[4][0], "two" );
	FAILNE( ta[5][0], "two" );
	FAILNE( ta[3][1], "1" );
	FAILNE( ta[4][1], "2" );
	FAILNE( ta[5][1], "3" );
}

DEFTEST( TwoFieldsDescTest ) {
	MakeData( rows2, ta );
	Sorter s;
	s.AddField( SortField( 0 ) );
	s.AddField( SortField( 1, SortField::dirDesc ) );
	s.Sort( ta );
	//Dump( ta );
	FAILNE( ta.size(), 6 );
	FAILNE( ta[3][0], "two" );
	FAILNE( ta[4][0], "two" );
	FAILNE( ta[5][0], "two" );
	FAILNE( ta[3][1], "3" );
	FAILNE( ta[4][1], "2" );
	FAILNE( ta[5][1], "1" );
}

#endif

// end
