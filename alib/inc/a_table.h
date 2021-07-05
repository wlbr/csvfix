//---------------------------------------------------------------------------
// a_table.h
//
// data table for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_TABLE_H
#define INC_A_TABLE_H

#include "a_base.h"

//---------------------------------------------------------------------------
// Begin ALib stuff
//---------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// Column name & type.
//---------------------------------------------------------------------------

class Column {

	public:

		enum ColumnType { String, Integer, Real, Date };

		Column( const std::string & name, ColumnType type );

		const std::string & Name() const;
		ColumnType Type() const;

	private:

		std::string mName;
		ColumnType mType;
};

//---------------------------------------------------------------------------
// Data values are always represented as strings and converted as needed.
// Conversions are not checked against column type.
// Values can be copied via default copy & assignment ops.
//---------------------------------------------------------------------------

class TableValue {

	public:
		
		TableValue();
		TableValue( const std::string & s );
		TableValue( int n );
		TableValue( double n );

		bool IsNull() const;

		const std::string & AsString() const;
		int AsInteger() const;
		double AsDouble() const;

	private:

		std::string mVal;
		bool mIsNull;
};

//---------------------------------------------------------------------------
// Row is a vector of values 
//---------------------------------------------------------------------------

typedef std::vector <TableValue> TableRow;

//---------------------------------------------------------------------------
// Table is a collection of rows, with a set of columns.
//---------------------------------------------------------------------------

class Table {

	public:

		Table();
		virtual ~Table();
		void Clear();

		void AddColumn( const Column & col );
		unsigned int ColumnCount() const;
		int ColumnIndex( const std::string & name ) const;
		const Column & ColumnAt( unsigned int i ) const;

		void AddRow( const TableRow & row );
		void RemoveRow( unsigned int  row );
		void ClearRows();
		
		const TableRow & Row( unsigned int row ) const;
		TableValue & Value( unsigned int col, unsigned int row ) const;

		unsigned int Depth() const;
		unsigned int Width() const;

		void DumpOn( std::ostream & os ) const;

	private:

		std::vector <Column> mCols;
		std::vector <TableRow> mRows;

};

//------------------------------------------------------------------------
// end ALib stuff
//------------------------------------------------------------------------

}


#endif

