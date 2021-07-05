//---------------------------------------------------------------------------
// a_db.cpp
//
// ODBC database connectivity for alib
// this is Windows only.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include <memory>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include "a_db.h"
#include "a_collect.h"
#include "a_str.h"


using std::string;
using std::vector;

namespace ALib {

//----------------------------------------------------------------------------
// default string to represent nulls
//----------------------------------------------------------------------------

const char * const DEF_NULL_STR = "NULL";

//----------------------------------------------------------------------------
// Check return codes of ODBC function and setb error & state accordingly
//----------------------------------------------------------------------------

class ODBCStatus {

	public:

		ODBCStatus();
		ODBCStatus( SQLRETURN res, DBCImpl & dbc );
		ODBCStatus( SQLRETURN res, DBSImpl & dbs );

		void Clear();

		bool OK() const;
		bool Failed() const;
		operator void *() const;

		string State() const;
		string Error() const;

	private:

		void DoCheck( SQLRETURN r, SQLHANDLE h, SQLSMALLINT ht );

		enum { STATESZ = 16, ERRSZ = 512 };
		char mState[STATESZ];
		char mError[ERRSZ];
};


//----------------------------------------------------------------------------
// Buffers for SELECT output
//----------------------------------------------------------------------------

class ODBCBuffer {

	CANNOT_COPY( ODBCBuffer );

	public:

		ODBCBuffer( const string & name, SQLSMALLINT type, int width );
		~ODBCBuffer();

		char * Buffer() const;
		const string & Name() const;
		int Width() const;
		SQLLEN & ReadWidth();
		SQLSMALLINT Type() const;

	private:

		string mName;
		SQLSMALLINT mType;
		int mWidth;
		char * mBuffer;
		SQLLEN mReadWidth;

};

//----------------------------------------------------------------------------
// Connection implementation
//----------------------------------------------------------------------------

class DBCImpl {

	public:

		DBCImpl();
		~DBCImpl();

		bool Connect( const std::string & cs );
		void Disconnect();
		bool IsConnected() const;

		static SQLHANDLE GetEnv();
		SQLHANDLE Handle() const;

		const ODBCStatus & Status() const;
		bool NextRow( DbRow & row );

	private:

		void AllocConnect();
		void FreeConnect();

		static SQLHANDLE mEnv;
		SQLHANDLE mConnect;
		ODBCStatus mStatus;
};


//----------------------------------------------------------------------------
// Statement implementation
//----------------------------------------------------------------------------

class DBSImpl {

	public:

		DBSImpl( DBCImpl * dbc  );
		~DBSImpl();

		void Clear();
		bool HasResultSet();
	    std::string Error() const;

		SQLHANDLE Handle() const;

		const ODBCStatus & Status() const;

		bool Execute( const string & sql );
		bool Fetch( DbRow & row );
		unsigned int ColumnCount() const;
		DbColumnInfo ColumnInfo( unsigned int i ) const;
		int ColumnIndex( const std::string & colname ) const;

		void SetNull( const string & nullstr );

	private:

		bool CreateBuffers();

		SQLHANDLE mStmt;
		ODBCStatus mStatus;
		unsigned int mColCount;
		vector <ODBCBuffer *> mBuffers;
		string mNullStr;
};

//----------------------------------------------------------------------------
// Helper to convert string to something ODBC will like
//----------------------------------------------------------------------------

static SQLCHAR * ODBCStr( const string & s ) {
	return (SQLCHAR *) s.c_str();
}

//----------------------------------------------------------------------------
// construct in OK state
//----------------------------------------------------------------------------

ODBCStatus :: ODBCStatus() {
	mState[0] = mError[0] = 0;
}

//----------------------------------------------------------------------------
// construct in state indicated by res - reports connect errors
//----------------------------------------------------------------------------

ODBCStatus :: ODBCStatus( SQLRETURN res, DBCImpl & dbc ) {
	DoCheck( res, dbc.Handle(), SQL_HANDLE_DBC );
}

//----------------------------------------------------------------------------
// construct in state indicated by res - reports exec errors
//----------------------------------------------------------------------------

ODBCStatus :: ODBCStatus( SQLRETURN res, DBSImpl & dbs ) {
	DoCheck( res, dbs.Handle(), SQL_HANDLE_STMT );
}

//----------------------------------------------------------------------------
// ODBC operation worked
//----------------------------------------------------------------------------

bool ODBCStatus :: OK() const {
	return mState[0] == 0;
}

//----------------------------------------------------------------------------
// operation failed
//----------------------------------------------------------------------------

bool ODBCStatus :: Failed() const {
	return ! OK();
}

//----------------------------------------------------------------------------
// for use in C++ conditional statements
//----------------------------------------------------------------------------

ODBCStatus :: operator void *() const {
	return OK() ? (void *) this : 0;
}

//----------------------------------------------------------------------------
// get state string - this contains whatever the ODBC driver set
//----------------------------------------------------------------------------

string ODBCStatus :: State() const {
	return mState;
}

//----------------------------------------------------------------------------
// get full errorv message for last op from ODBC driver
//----------------------------------------------------------------------------

string ODBCStatus :: Error() const {
	return mError;
}

//----------------------------------------------------------------------------
// reset back to OK state
//----------------------------------------------------------------------------

void ODBCStatus :: Clear() {
	mError[0] = mState[0] = 0;
}

//----------------------------------------------------------------------------
// check return code & set error & state messages
// note odbc3 allows multiple diagnostics, but we just get first one
//----------------------------------------------------------------------------

void ODBCStatus :: DoCheck( SQLRETURN r, SQLHANDLE h, SQLSMALLINT ht ) {

	if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
		mError[0] = mState[0] = 0;
		return;				// ok!
	}

	SQLINTEGER native;
	SQLSMALLINT ignore;

	if ( SQLGetDiagRec( ht, h, 1,
						(unsigned char *) mState,
						& native,
						(unsigned char *) mError, ERRSZ,
						& ignore )  != SQL_SUCCESS ) {
		ATHROW( "ODBC call failed - could not retrieve diagnostic" );
	}
}

//----------------------------------------------------------------------------
// construct buffer of suitable size
//----------------------------------------------------------------------------

ODBCBuffer :: ODBCBuffer( const string & name, SQLSMALLINT type, int width )
				: mName( name ), mType( type ), mWidth( width ), mBuffer(0) {
	mBuffer = new char[ mWidth ];
}

ODBCBuffer :: ~ODBCBuffer() {
	delete [] mBuffer;
}

//----------------------------------------------------------------------------
// accessors
//----------------------------------------------------------------------------

char * ODBCBuffer :: Buffer() const {
	return mBuffer;
}

const string & ODBCBuffer :: Name() const {
	return mName;
}

int ODBCBuffer :: Width() const {
	return mWidth;
}

//----------------------------------------------------------------------------
// return the width of data froma select or null indication
// must return reference as is used in buffer binding
//----------------------------------------------------------------------------

SQLLEN & ODBCBuffer :: ReadWidth() {
	return mReadWidth;
}

//----------------------------------------------------------------------------
// sql column type - the values are defined in sql.h
//----------------------------------------------------------------------------

SQLSMALLINT ODBCBuffer :: Type() const {
	return mType;
}

//----------------------------------------------------------------------------
// shared ODBC environment
//----------------------------------------------------------------------------

SQLHANDLE DBCImpl :: mEnv = 0;

//----------------------------------------------------------------------------
// construct in disconnected state
//----------------------------------------------------------------------------

DBCImpl :: DBCImpl() : mConnect( 0 ) {
}

DBCImpl :: ~DBCImpl() {
	FreeConnect();
}

//----------------------------------------------------------------------------
// Get underlying connection handle
//----------------------------------------------------------------------------

SQLHANDLE DBCImpl :: Handle() const {
	return mConnect;
}

//----------------------------------------------------------------------------
// Connect using standard ODBC connection string. we make no attempt
// to parse or check the connection string.
//----------------------------------------------------------------------------

bool DBCImpl :: Connect( const string & cs ) {

	AllocConnect();

	const SQLSMALLINT BUFSIZE = 1024;
	SQLSMALLINT inlen = cs.length();
	SQLSMALLINT outlen;
	unsigned char buffer[ BUFSIZE ];

	mStatus = ODBCStatus( SQLDriverConnect( mConnect, NULL,
									ODBCStr( cs ),inlen,
									buffer, BUFSIZE,& outlen,
									SQL_DRIVER_NOPROMPT ), *this );

	if ( mStatus.OK() ) {
		return true;
	}
	else {
		FreeConnect();
		return false;
	}
}

//----------------------------------------------------------------------------
// Connection stuff
//----------------------------------------------------------------------------

void DBCImpl :: Disconnect() {
	FreeConnect();
}

bool DBCImpl :: IsConnected() const {
	return mConnect != 0;
}

const ODBCStatus & DBCImpl :: Status() const {
	return mStatus;
}

//----------------------------------------------------------------------------
// get environment handle, allocating if needed - any error here is
// considered to be fatal
//----------------------------------------------------------------------------

SQLHANDLE  DBCImpl :: GetEnv() {
	if ( mEnv != 0 ) {
		return mEnv;
	}
	if ( SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, & mEnv )
						!= SQL_SUCCESS ) {
		ATHROW( "Cannot allocate ODBC environment handle" );
	}
	long long ver = SQL_OV_ODBC2;
	if ( SQLSetEnvAttr( mEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)ver , 0 )
							!= SQL_SUCCESS ) {
		ATHROW( "Cannot set ODBC version" );
	}

	return mEnv;
}

//----------------------------------------------------------------------------
// allocate a connection handle, freeing any existing one
// failure to alklocate is treated as fatal
//----------------------------------------------------------------------------

void DBCImpl :: AllocConnect() {
	FreeConnect();
	if ( SQLAllocHandle( SQL_HANDLE_DBC, GetEnv(), & mConnect ) != SQL_SUCCESS ) {
		ATHROW( "Could not allocate ODBC connection handle" );
	}
}

//----------------------------------------------------------------------------
// free connection handle, if one exists. free is assumed always to work
//----------------------------------------------------------------------------

void DBCImpl :: FreeConnect() {
	if ( mConnect ) {
		SQLFreeHandle( SQL_HANDLE_DBC, mConnect );
		mConnect = 0;
	}
}

//----------------------------------------------------------------------------
// Statement implementation - must have connection
//----------------------------------------------------------------------------

DBSImpl :: DBSImpl( DBCImpl * dbc )
			: mStmt( 0 ), mColCount(0), mNullStr( DEF_NULL_STR ) {

	if ( ! dbc->IsConnected() ) {
		ATHROW( "Cannot create statement on unconnected database" );
	}
	if ( SQLAllocHandle( SQL_HANDLE_STMT, dbc->Handle(), & mStmt ) != SQL_SUCCESS ) {
		ATHROW( "Could not allocate ODBC statement handle" );
	}

}

DBSImpl :: ~DBSImpl() {
	SQLFreeHandle( SQL_HANDLE_STMT, mStmt );
}

SQLHANDLE DBSImpl :: Handle() const {
	return mStmt;
}

//----------------------------------------------------------------------------
// set string to use to represent nulls
//----------------------------------------------------------------------------

void DBSImpl :: SetNull( const string & nullstr ) {
	mNullStr = nullstr;
}

//----------------------------------------------------------------------------
// remove any existing results
//----------------------------------------------------------------------------

void DBSImpl :: Clear() {
	mColCount = 0;
	ALib::FreeClear( mBuffers );
}

//----------------------------------------------------------------------------
// last error
//----------------------------------------------------------------------------

string DBSImpl :: Error() const {
	return mStatus.Error();
}

//----------------------------------------------------------------------------
// number of columns - will be zero for non-SELECT statements
//----------------------------------------------------------------------------

unsigned int DBSImpl :: ColumnCount() const {
	return mColCount;
}

//----------------------------------------------------------------------------
// status of last execed command
//----------------------------------------------------------------------------

const ODBCStatus & DBSImpl :: Status() const {
	return mStatus;
}

//----------------------------------------------------------------------------
// execute command - return true on success
//----------------------------------------------------------------------------

bool DBSImpl :: Execute( const string & sql ) {
	Clear();
	mStatus = ODBCStatus(
				SQLExecDirect( mStmt, ODBCStr( sql ), sql.size() )
				, *this );
	if ( mStatus.OK() ) {
		SQLSMALLINT ncols;
	    mStatus = ODBCStatus( SQLNumResultCols( Handle(), & ncols ), *this );
		if ( mStatus.OK() ) {
			mColCount = ncols;
			return CreateBuffers();
		}
	}

	return false;
}

//----------------------------------------------------------------------------
// helper tyo get suitable buffer size for an ODBC type
//----------------------------------------------------------------------------

static int BufferSize( SQLSMALLINT type ) {
	switch( type ) {
		case SQL_DATETIME:
		case SQL_DECIMAL:
		case SQL_FLOAT:
		case SQL_INTEGER:
		case SQL_NUMERIC:
		case SQL_REAL:
		case SQL_SMALLINT:
		case SQL_TINYINT: 		return 16;
		default:				return 2048;
	}
}

//----------------------------------------------------------------------------
// create all buffers for a query that returnd results and bind them
// to the statement
//----------------------------------------------------------------------------

bool DBSImpl :: CreateBuffers() {
	const int NAMEBUFSIZE = 256;
	SQLCHAR name[NAMEBUFSIZE];
	SQLSMALLINT namelen, digits, nulls, coltype;
	SQLULEN colwidth;

	for ( unsigned int col = 0; col < mColCount; col++ ) {

		mStatus = ODBCStatus(
		             SQLDescribeCol( Handle(), col + 1, name, NAMEBUFSIZE,
										& namelen, & coltype,
										& colwidth, & digits, & nulls ), *this );
		if ( mStatus.Failed() ) {
			return false;
		}

		string sname((char *)name );
		int blen = BufferSize( coltype );
		std::auto_ptr <ODBCBuffer> buf( new ODBCBuffer( sname, coltype, blen  ) );
		mStatus = ODBCStatus( SQLBindCol( Handle(), col + 1, SQL_C_CHAR, buf->Buffer(),
								blen, & buf->ReadWidth() ), *this );
		if ( mStatus.Failed() ) {
			return false;
		}

		mBuffers.push_back( buf.release() );

	}
	return true;
}

//----------------------------------------------------------------------------
// translate ODBC col types to those used by our interface
//----------------------------------------------------------------------------

static DbColumnInfo::ColType TransColType( SQLSMALLINT type ) {
	switch( type ) {
		case SQL_DATETIME:	return DbColumnInfo::ctDate;
		case SQL_DECIMAL:
		case SQL_FLOAT:
		case SQL_NUMERIC:
		case SQL_REAL:		return DbColumnInfo::ctReal;
		case SQL_SMALLINT:
		case SQL_TINYINT:
		case SQL_INTEGER:	return DbColumnInfo::ctInt;
		case SQL_CHAR:
		case SQL_VARCHAR:	return DbColumnInfo::ctStr;
		default:			return DbColumnInfo::ctBlob;
	}
}

//----------------------------------------------------------------------------
// get info about column from result
//----------------------------------------------------------------------------

DbColumnInfo DBSImpl :: ColumnInfo( unsigned int i ) const {
	if ( i >= mColCount ) {
		ATHROW( "Invalid column index: " << i );
	}
	DbColumnInfo::ColType type = TransColType( mBuffers[i]->Type() );
	return DbColumnInfo( mBuffers[i]->Name(), type, mBuffers[i]->ReadWidth() );
}

//----------------------------------------------------------------------------
// Get column index of named column, or -1 if not found
//----------------------------------------------------------------------------

int DBSImpl :: ColumnIndex( const string & colname ) const {
	for ( unsigned int i = 0; i < mColCount; i++ ) {
		DbColumnInfo ci = ColumnInfo( i );
		if ( Equal( ci.Name(), colname ) ) {
			return i;
		}
	}
	return -1;
}

//----------------------------------------------------------------------------
// fetch next row - returns true if row was retrieved
// will return false on no more rows or on error
//----------------------------------------------------------------------------

bool DBSImpl :: Fetch( DbRow & row ) {
	if ( mColCount == 0 ) {
		return false;
	}
	row.clear();
	mStatus.Clear();
	SQLRETURN rv = SQLFetch( Handle() );
	if ( rv == SQL_NO_DATA ) {
		return false;
	}
	else {
		mStatus = ODBCStatus( rv, *this );
		if ( mStatus.Failed() ) {
			return false;
		}
		for ( unsigned int i = 0; i < mColCount; i++ ) {
			if ( mBuffers[i]->ReadWidth() == SQL_NULL_DATA ) {
				row.push_back( mNullStr );
			}
			else {
				row.push_back( mBuffers[i]->Buffer() );
			}
		}
		return true;
	}
}
//----------------------------------------------------------------------------
// Public connection class simply forwards to implementation
//----------------------------------------------------------------------------

DbConnection :: DbConnection( const string & cs )
	: mImpl( new DBCImpl ) {
}

DbConnection :: ~DbConnection() {
	delete mImpl;
}

bool DbConnection :: Connect( const std::string & cs ) {
	return mImpl->Connect( cs );
}

void DbConnection :: Disconnect() {
	mImpl->Disconnect();
}

bool DbConnection :: IsConnected() const {
	return mImpl->IsConnected();
}

string DbConnection :: Error() const {
	return mImpl->Status().Error();
}


DbColumnInfo :: DbColumnInfo( const string & name, ColType t, int sz )
	: mName( name ), mType( t ), mSize( sz ) {
}


string DbColumnInfo :: Name() const {
	return mName;
}

DbColumnInfo::ColType DbColumnInfo :: Type() const {
	return mType;
}

int DbColumnInfo :: Size() const {
	return mSize;
}

//----------------------------------------------------------------------------
// Public statement class forwards to implementation
//----------------------------------------------------------------------------

DbStatement :: DbStatement( DbConnection & dbc ) : mImpl( 0 ) {
	mImpl = new DBSImpl( dbc.mImpl );
}

DbStatement :: ~DbStatement() {
	delete mImpl;
}

void DbStatement :: Clear() {
	mImpl->Clear();
}

bool DbStatement :: HasResultSet() {
	return false;
}

string DbStatement :: Error() const {
	return mImpl->Status().Error();
}

bool DbStatement :: Exec( const string & sql ) {
	return mImpl->Execute( sql );
}

int DbStatement :: ColumnCount() const {
	return mImpl->ColumnCount();
}

DbColumnInfo DbStatement :: ColumnInfo( int idx ) const {
	return mImpl->ColumnInfo( idx );
}

bool DbStatement :: Fetch( DbRow & row ) {
	return mImpl->Fetch( row );
}

void DbStatement :: SetNull( const string & nullstr ) {
	mImpl->SetNull( nullstr );
}

//----------------------------------------------------------------------------

}	// end namespace

//----------------------------------------------------------------------------
// Testing - requires a sqlite database to work on
//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_db" );

DEFTEST( ConnectTest ) {
	DbConnection db;
	bool ok = db.Connect( "bad connect" );
	FAILNE( ok, false );
	FAILNE( db.IsConnected(), false );
	string cs = "Driver=SQLite3 ODBC Driver;Database=db\\test.dat;";
	ok = db.Connect( cs );
	FAILNE( ok, true );
	FAILNE( db.IsConnected(), true );
	db.Disconnect();
	FAILNE( db.IsConnected(), false );
}

const char * const TBL_SQL1 =
	"DROP TABLE foo";

const char * const TBL_SQL2 =
	"CREATE TABLE foo ( anum, name )";

const char * const INS_SQL1 =
	"INSERT INTO foo VALUES ( 1, 'one' )";
const char * const INS_SQL2 =
	"INSERT INTO foo VALUES ( 2, NULL )";



DEFTEST( StmtTest ) {
	DbConnection db;
	string cs = "Driver=SQLite3 ODBC Driver;Database=db\\test.dat;";
	bool ok = db.Connect( cs );
	FAILNE( ok, true );
	DbStatement stmt( db );
	ok = stmt.Exec( "DELETE FROM xxxxx" );
	FAILNE( ok, false );
	FAILEQ( stmt.Error(), "" );
	ok = stmt.Exec( TBL_SQL1 );
	ok = stmt.Exec( TBL_SQL2 );
	FAILNE( ok, true );
	ok = stmt.Exec( INS_SQL1 );
	FAILNE( ok, true );
	ok = stmt.Exec( INS_SQL2 );
	FAILNE( ok, true );
	ok = stmt.Exec( "SELECT * FROM foo" );
	FAILNE( ok, true );
	int cols = stmt.ColumnCount();
	FAILNE( cols, 2 );
	DbColumnInfo ci = stmt.ColumnInfo( 0 );
	FAILNE( ci.Name(), "anum" );
	DbRow row;
	ok = stmt.Fetch( row );
	FAILNE( ok, true );
	FAILNE( row.size(), 2 );
	FAILNE( row[0], "1" );
	FAILNE( row[1], "one" );
//	Dump( cout, row, "" );
	ok = stmt.Fetch( row );
	FAILNE( ok, true );
	FAILNE( row.size(), 2 );
	FAILNE( row[0], "2" );
	FAILNE( row[1], "NULL" );
//	Dump( cout, row, "" );
}


#endif


