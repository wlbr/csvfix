//---------------------------------------------------------------------------
// csved_odbc.cpp
//
// exec SQL commands agains database producing CSV output
// this is Windows only, at present
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include <assert.h>
#include <fstream>
#include "a_base.h"
#include "a_file.h"
#include "csved_cli.h"
#include "csved_odbc.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//----------------------------------------------------------------------------
// Register command
//----------------------------------------------------------------------------

static RegisterCommand <ODBCGetCommand> rc1_(
	CMD_ODBCGET,
	"get data from ODBC database"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const OGET_HELP = {
	"extract data from ODBC-connected database and format as CSV\n"
	"note that this command does not read input streams\n"
	"usage: csvfix odbc_get  [flags] [file ...]\n"
	"where flags are:\n"
	"  -cs cstr\tODBC connection string\n"
	"  -sql stmt\tSQL statement to execute to extract data\n"
	"  -tbl table\textract all data from table  rather than use SQL\n"
	"  -ns null\tstring to use to represent nulls (default is empty string)\n"
	"  -h\t\toutput SQL column headers as CSV field name header\n"
	"#SMQ,OFL"
};

//----------------------------------------------------------------------------
// base command ctor
//----------------------------------------------------------------------------

ODBCGetCommand :: ODBCGetCommand( const string & name,
								const string & desc )
	: Command( name, desc, OGET_HELP ), mStatement( 0 ), mUseColNames( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_SQLQ, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_SQLTBL, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_CONSTR, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_NULLSTR, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_DIR, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_HEADER, false, 0 ) );
}

//----------------------------------------------------------------------------
// Junk statement
//----------------------------------------------------------------------------

ODBCGetCommand :: ~ODBCGetCommand() {
	delete mStatement;
}

//----------------------------------------------------------------------------
// connect to database using connection string
//----------------------------------------------------------------------------

void ODBCGetCommand :: Connect() {
	delete mStatement;
	mStatement = 0;
	if ( ! mConnection.Connect( mConnStr ) ) {
		CSVTHROW( "Database connection failed: " << mConnection.Error() );
	}
	mStatement = new ALib::DbStatement( mConnection );
}

//----------------------------------------------------------------------------
// execute any SQL command
//----------------------------------------------------------------------------

void ODBCGetCommand :: Exec( const string & sql ) {
	assert( mStatement );
	if ( ! mStatement->Exec( sql ) ) {
		CSVTHROW( "SQL error: " << mStatement->Error() );
	}
}

//----------------------------------------------------------------------------
// get statement
//----------------------------------------------------------------------------

ALib::DbStatement * ODBCGetCommand :: Stmt() {
	return mStatement;
}

//----------------------------------------------------------------------------
// Flags now considerably complicated by addition of the -dir parameter, which
// requires we synthesise a connection string to use the ODBC text driver.
//----------------------------------------------------------------------------

const string TEXT_DRIVER = "DRIVER={Microsoft Text Driver (*.txt; *.csv)};";

void ODBCGetCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	mUseColNames = cmd.HasFlag( FLAG_HEADER );
	NotBoth( cmd, FLAG_DIR, FLAG_CONSTR, ReqOp::Required );

	if ( cmd.HasFlag( FLAG_DIR ) ) {
		string dir = cmd.GetValue( FLAG_DIR, "" );
		if ( dir == "" ) {
			CSVTHROW( "Directory string cannot be empty" );
		}
		if ( ! ALib::DirExists( dir ) ) {
			CSVTHROW( "No such directory as " << dir );
		}
		mConnStr = TEXT_DRIVER + "DEFAULTDIR=" + dir + ";";
	}
	else {
		mConnStr = cmd.GetValue( FLAG_CONSTR, "" );
		if ( mConnStr == "" ) {
			CSVTHROW( "Connection string cannot be empty" );
		}
	}

	NotBoth( cmd, FLAG_SQLTBL, FLAG_SQLQ, ReqOp::Required );

	if ( cmd.HasFlag( FLAG_SQLTBL ) ) {
		string table = cmd.GetValue( FLAG_SQLTBL, "" );
		if ( table == "" ) {
			CSVTHROW( "Table name required" );
		}
		mSql = "SELECT * FROM [" + table + "]";
	}
	else if ( cmd.HasFlag( FLAG_SQLQ ) ) {
		mSql = cmd.GetValue( FLAG_SQLQ, "" );
		if ( mSql == "" ) {
			CSVTHROW( "SQL statement cannot be empty" );
		}
		if ( mSql[0] == '@' ) {
			string file = mSql.substr(1);
			mSql = "";
			ALib::FileRead( file, mSql );
		}
	}

	mNull = cmd.GetValue( FLAG_NULLSTR, "NULL" );
}


//----------------------------------------------------------------------------
// Connect, exec SQL, and then read all rows converting to csv.
// Added column names as header. AFAIK, SQL column names will not contain
// special characters, so no quoting needed.
//----------------------------------------------------------------------------

int ODBCGetCommand :: Execute( ALib::CommandLine & cmd ) {
	ProcessFlags( cmd );

	Connect();;
	Exec( mSql );

	ALib::DbRow row;
	IOManager io( cmd );

	Stmt()->SetNull( mNull );

	if ( mUseColNames ) {
		int colcount = Stmt()->ColumnCount();
		for ( int i = 0; i < colcount; i++ ) {
			if ( i ) {
				io.Out() << ",";
			}
			io.Out() << Stmt()->ColumnInfo(i).Name();
		}
		io.Out() << "\n";
	}

	while( Stmt()->Fetch( row ) ) {
		io.WriteRow( row );
	}
	return 0;
}

//----------------------------------------------------------------------------

} // end namespace

// end

