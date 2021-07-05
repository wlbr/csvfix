//---------------------------------------------------------------------------
// a_db.h
//
// database connectivity for alib
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_DB_H
#define INC_A_DB_H

#include "a_base.h"

namespace ALib {

//------------------------------------------------------------------------
// TheDbConnection class is intended to provide implementation
// independant database access to SQL databases. All the other database
// classes in ALib require a DbConnection to be established before they
// can be used.
//
// Connection parameters are specified by a connection string. The default
// implementation supplied by ALib uses a standard ODBC connection string.
//----------------------------------------------------------------------------

class DbConnection {

	friend class DbStatement;
	CANNOT_COPY( DbConnection );

	public:

		DbConnection( const std::string & cs = "" );
		virtual ~DbConnection();

		bool Connect( const std::string & cs );
		void Disconnect();
		bool IsConnected() const;

		std::string Error() const;

	private:

		class DBCImpl * mImpl;
};

//------------------------------------------------------------------------
// Provides information regarding columns in database query results. The
// only currently supplied information is the column type and name
//----------------------------------------------------------------------------

class DbColumnInfo {

	public:

		// Types are a subset of the actual datbnase types
		enum ColType { ctStr, ctInt, ctReal, ctDate, ctBlob };

		DbColumnInfo( const std::string & name, ColType t, int sz );
		std::string Name() const;
		ColType Type() const;
		int Size() const;

	private:

		std::string mName;
		ColType mType;
		int mSize;
};

//------------------------------------------------------------------------
// Row data is always representwed as strings, no matterw hat the
// actual type in the database.
//----------------------------------------------------------------------------

typedef std::vector <std::string> DbRow;

//------------------------------------------------------------------------
// This class allows execution of commands and retrieval of result sets.
// A connected database is required in order to use dbStatement.
//----------------------------------------------------------------------------

class DbStatement {

	CANNOT_COPY( DbStatement );

	public:

		DbStatement( DbConnection & dbc );
		virtual ~DbStatement();

		void Clear();

		bool HasResultSet();

	    std::string Error() const;

		int ColumnCount() const;
		DbColumnInfo ColumnInfo( int idx ) const;
		int ColumnIndex( const std::string & name ) const;

		void SetNull( const std::string & nullstr );

		bool Fetch( DbRow & row );
		bool Exec( const std::string & sql );

	private:

		class DBSImpl * mImpl;

};

//------------------------------------------------------------------------

} // namespace

#endif

