//---------------------------------------------------------------------------
// a_table.cpp
//
// data table for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_except.h"
#include "a_table.h"
#include "a_str.h"
using std::string;
using std::vector;


//---------------------------------------------------------------------------
// Begin ALib stuff
//---------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// Columns have name and type. Types are based on those of standard SQL, with
// binary (blob) types being represenred by strings. Name may be empty.
//---------------------------------------------------------------------------

Column :: Column( const string & name, ColumnType type )
	: mName( name ), mType( type ) {
}

const string & Column :: Name() const {
	return mName;
}

Column::ColumnType Column :: Type() const {
	return mType;
}

//---------------------------------------------------------------------------
// Tables are built incrementally by adding columns and then rows. Once a
// column has a row, more columns cannot be added.
//---------------------------------------------------------------------------

Table :: Table() {
	// nothing
}

Table :: ~Table() {
	Clear();
}

//---------------------------------------------------------------------------
// Remove everything
//---------------------------------------------------------------------------

void Table :: Clear() {
	mCols.clear();
	ClearRows();
}

//---------------------------------------------------------------------------
// Add column. Columns do not have to have name and if named the name need
// not be unique. However, only the frst non-uniquely named columns can be
// retrieved by name.
//---------------------------------------------------------------------------

void Table :: AddColumn( const Column & col ) {
	mCols.push_back( col );
}

//---------------------------------------------------------------------------
// How many columns?
//---------------------------------------------------------------------------

unsigned int Table :: ColumnCount() const {
	return mCols.size();
}

//---------------------------------------------------------------------------
// Return index of named column, or -1 if does not exist
//---------------------------------------------------------------------------

int Table :: ColumnIndex( const std::string & name ) const {
	for ( unsigned int i = 0; i < mCols.size(); i++ ) {
		if ( Equal( name, mCols[i].Name()) ) {
			return i;
		}
	}
	return -1;
}

//---------------------------------------------------------------------------
// Add row to table
//---------------------------------------------------------------------------

void Table :: AddRow( const TableRow & row ) {
	if ( row.size() != mCols.size() ) {
		ATHROW( "Row and columns sizes do not match" );
	}
	mRows.push_back( row );
}

//---------------------------------------------------------------------------
// Remove all rows
//---------------------------------------------------------------------------

void Table :: ClearRows() {
	mRows.clear();
}

//---------------------------------------------------------------------------
// Get ref to row at row index
//---------------------------------------------------------------------------

const TableRow & Table :: Row( unsigned int row ) const {
	if ( row >= mRows.size() ) {
		ATHROW( "Row index " << row << " out of range" );
	}
	return mRows[row];
}

//---------------------------------------------------------------------------
// Het value at column/row
//---------------------------------------------------------------------------

TableValue & Table :: Value( unsigned int col, unsigned int row ) const {
	if ( col >= mCols.size() ) {
		ATHROW( "Column index " << col << " out of range" );
	}
	return const_cast <TableRow&>(Row( row ))[col];
}

//---------------------------------------------------------------------------
// Depth is number of rows
//---------------------------------------------------------------------------

unsigned int Table :: Depth() const {
	return mRows.size();
}

//---------------------------------------------------------------------------
// Width is number of columns
//---------------------------------------------------------------------------

unsigned int Table :: Width() const {
	return mCols.size();
}

//---------------------------------------------------------------------------
// Get column at index or throw if not there
//---------------------------------------------------------------------------

const Column &  Table :: ColumnAt( unsigned int i ) const {
	if ( i >= mCols.size() ) {
		ATHROW( "Column index " << i << " out of range" );
	}
	return mCols[i];
}

//---------------------------------------------------------------------------
// Debug dump
//---------------------------------------------------------------------------

void Table :: DumpOn( std::ostream & os ) const {

	for ( unsigned int ci = 0; ci < Width(); ci++ ) {
		if ( ci != 0 ) {
			os << "|";
		}
		os << mCols[ci].Name();
	}
	os << "\n";

	for ( unsigned int ri = 0; ri < Depth(); ri++ ) {
		for ( unsigned int ci = 0; ci < Width(); ci++ ) {
			if ( ci != 0 ) {
				os << "|";
			}
			os << Value( ci, ri ).AsString();
		}
		os << "\n";
	}
}

//---------------------------------------------------------------------------
// Create table values with value and null indicator
//---------------------------------------------------------------------------

TableValue :: TableValue()
	: mVal( "" ), mIsNull( true ) {
}

TableValue :: TableValue( const string & s )
	: mVal( s ), mIsNull( false ) {
}

TableValue :: TableValue( int n )
	: mVal( Str(n) ), mIsNull( false ) {
}

TableValue :: TableValue( double n )
	: mVal( Str(n) ), mIsNull( false ) {
}

//---------------------------------------------------------------------------
// Is this value a null?
//---------------------------------------------------------------------------

bool TableValue :: IsNull() const {
	return mIsNull;
}

//---------------------------------------------------------------------------
// Nulls have string rep which is the string "NULL"
//---------------------------------------------------------------------------

const std::string & TableValue :: AsString() const {
	if ( mIsNull ) {
		static string NullVal = "NULL";
		return NullVal;
	}
	else {
		return mVal;
	}
}

//---------------------------------------------------------------------------
// Null integers don't  have rep
//---------------------------------------------------------------------------

int TableValue :: AsInteger() const {
	if ( mIsNull ) {
		ATHROW( "Null cannot be converted to integer" );
	}
	else {
		return ToInteger( mVal );
	}
}

//---------------------------------------------------------------------------
// Null reals don't	have rep
//---------------------------------------------------------------------------

double TableValue :: AsDouble() const {
	if ( mIsNull ) {
		ATHROW( "Null cannot be converted to real" );
	}
	else {
		return ToReal( mVal );
	}
}

//----------------------------------------------------------------------------

} // end namespace



//----------------------------------------------------------------------------
// Unit tests
//----------------------------------------------------------------------------

#ifdef ALIB_TEST

#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_table" );

DEFTEST( EmptyTable ) {
	Table t;
	FAILIF( t.Depth() != 0 );
	FAILIF( t.Width() != 0 );
}

DEFTEST( AddColToTable ) {
	Table t;
	t.AddColumn( Column( "name", Column::String ));
	t.AddColumn( Column( "number", Column::Integer ));
	FAILIF( t.Depth() != 0 );
	FAILIF( t.Width() != 2 );
	FAILIF( t.ColumnIndex( "name") != 0 );
	FAILIF( t.ColumnIndex( "number") != 1 );
}

DEFTEST( AddColDataToTable ) {
	Table t;
	t.AddColumn( Column( "name", Column::String ));
	t.AddColumn( Column( "number", Column::Integer ));
	TableRow row;
	row.push_back( TableValue( "foo"));
	row.push_back( TableValue( "123"));
	t.AddRow( row );
	FAILIF( t.Depth() != 1 );
	FAILIF( t.Width() != 2 );
	FAILIF( t.Value(0,0).AsString() != "foo" );
	FAILIF( t.Value(1,0).AsInteger() != 123 );
}


#endif
