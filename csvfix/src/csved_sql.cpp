//---------------------------------------------------------------------------
// csved_sql.cpp
//
// generate SQL statements from CSV
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "a_sort.h"
#include "csved_cli.h"
#include "csved_sql.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register sql generation commands
//---------------------------------------------------------------------------

static RegisterCommand <SQLInsertCommand> rc1_(
	CMD_SQLIN,
	"generate SQL INSERT statements"
);

static RegisterCommand <SQLUpdateCommand> rc2_(
	CMD_SQLUP,
	"generate SQL UPDATE statements"
);

static RegisterCommand <SQLDeleteCommand> rc3_(
	CMD_SQLDEL,
	"generate SQL DELETE statements"
);


//----------------------------------------------------------------------------
// Help texts
//----------------------------------------------------------------------------

const char * const INS_HELP = {
	"generates SQL INSERT statements from  CSV data\n"
	"usage: csvfix sql_insert [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to use to generate the SQL statement\n"
	"  -t table\tname of table to insert into\n"
	"  -s sep\tspecifies statement separator (default ';')\n"
	"  -nq fields\tspecifies fields not to quote on output\n"
	"  -qn\t\tforce quoting of NULL values\n"
	"  -en\t\tcovert empty CSV fields to NULLs\n"
	"#IBN,SEP,OFL,IFN,SKIP"
};

const char * const UPD_HELP = {
	"generates SQL UPDATE statements from  CSV data\n"
	"usage: csvfix sql_update [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to use to generate the SQL SET clause\n"
	"  -w fields\tfields to use to generate the SQL WHERE clause\n"
	"  -t table\tname of table to update\n"
	"  -s sep\tspecifies statement separator (default ';')\n"
	"  -nq fields\tspecifies fields not to quote on output\n"
	"  -qn\t\tforce quoting of NULL values\n"
	"  -en\t\tcovert empty CSV fields to NULLs\n"
	"#IBN,SEP,OFL,IFN,SKIP"
};

const char * const DEL_HELP = {
	"generates SQL DELETE statements from  CSV data\n"
	"usage: csvfix sql_delete [flags] [file ...]\n"
	"where flags are:\n"
	"  -w fields\tfields to use to generate the SQL WHERE clause\n"
	"  -t table\tname of table to update\n"
	"  -s sep\tspecifies statement separator (default ';')\n"
	"  -nq fields\tspecifies fields not to quote on output\n"
	"  -qn\t\tforce quoting of NULL values\n"
	"  -en\t\tcovert empty CSV fields to NULLs\n"
	"#IBN,SEP,OFL,IFN,SKIP"
};

//---------------------------------------------------------------------------
// Base class for sql commands. Register common flags.
//---------------------------------------------------------------------------

SQLCommand :: SQLCommand( const string & name,
							const string & desc,
							const string & help )
	: Command( name, desc, help ), mQuoteNulls(false) {
	AddFlag( ALib::CommandLineFlag( FLAG_NOQUOTE, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_QNULLS, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_TABLE, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_SQLSEP, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ENULLS, false, 0 ) );

}

//---------------------------------------------------------------------------
// All commands need table name, separator to put at end of statement and
// list of fields not to quote.
//---------------------------------------------------------------------------

void SQLCommand :: GetCommonValues( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	mTable = cmd.GetValue( FLAG_TABLE );
	if ( mTable == "" ) {
		CSVTHROW( "Need table name specified by " << FLAG_TABLE << " flag" );
	}
	if ( ! cmd.HasFlag( FLAG_SQLSEP ) ) {
		mSep ="\n;\n";
	}
	else {
		mSep = ALib::UnEscape( cmd.GetValue( FLAG_SQLSEP ) );
	}

	string noq = cmd.GetValue( FLAG_NOQUOTE, "" );
	if ( ! ALib::IsEmpty( noq ) ) {
		CommaListToIndex( ALib::CommaList( noq ), mNoQuote );
	}
	mQuoteNulls = cmd.HasFlag( FLAG_QNULLS );
	mEmptyNulls = cmd.HasFlag( FLAG_ENULLS );
}

//---------------------------------------------------------------------------
// should we do sql quoting of this index?
//---------------------------------------------------------------------------

bool SQLCommand :: DoSQLQuote( unsigned int i ) const {
	return ALib::Contains( mNoQuote, i ) == false;
}

//----------------------------------------------------------------------------
// See if string should be quoted with regard to -qn setting.
//----------------------------------------------------------------------------

bool SQLCommand :: NoNullQuote( const string & ns ) const {
	if ( ALib::Equal( ns, "NULL" ) ) {
		return ! mQuoteNulls;
	}
	else {
		return false;
	}
}

//----------------------------------------------------------------------------
// Convert empty fields to NULL
//----------------------------------------------------------------------------

string SQLCommand :: EmptyToNull( const string & f ) const {
	if ( mEmptyNulls && ALib::IsEmpty( f ) ) {
		return "NULL";
	}
	else {
		return f;
	}

}

//---------------------------------------------------------------------------
// Create data and where columns from values specified by the relevant
// command line flags
//---------------------------------------------------------------------------

void SQLCommand :: BuildDataCols( const ALib::CommandLine & cmd ) {
	BuildCols( cmd, FLAG_COLS, mDataCols );
}

void SQLCommand :: BuildWhereCols( const ALib::CommandLine & cmd ) {
	BuildCols( cmd, FLAG_WHERE, mWhereCols );
}

//---------------------------------------------------------------------------
// Get the field specifications. These are in the form:
//
//		field_index:col_name
//
// where the name is optional. If no fields are specified, all
// fields in input rows are output.
//---------------------------------------------------------------------------

void SQLCommand :: BuildCols( const ALib::CommandLine & cmd,
								const string & flag,
								SQLColSpec::Vec & cols ) {

	bool havenames = false;
	cols.clear();

	if ( cmd.FlagCount( flag ) > 1 ) {
		CSVTHROW( "Need fields specified by single " << flag << " flag" );
	}

	ALib::CommaList cl( cmd.GetValue( flag ) );

	for ( unsigned int i = 0; i < cl.Size(); i++ ) {

		vector <string> tmp;

		if ( ALib::Split( cl.At(i), ':', tmp ) > 2 ) {
			CSVTHROW( "Invalid column specification: " << cl.At(i) );
		}

		if ( tmp.size() == 0 ) {
			CSVTHROW( "Empty column specification" );
		}

		if ( ! ALib::IsInteger( tmp[0] ) ) {
			CSVTHROW( "Field index must be integer in "  << cl.At(i) );
		}

		int icol = ALib::ToInteger( tmp[0] );
		if ( icol <= 0 ) {
			CSVTHROW( "Field index must be greater than xero in " << cl.At(i) );
		}

		if ( tmp.size() == 1 && havenames ) {
			CSVTHROW( "Must specify all column names" );
		}

		havenames = havenames || tmp.size() == 2;

		string colname = tmp.size() == 2 ? tmp[1] : string("");

		cols.push_back( SQLColSpec( icol - 1, colname ) );
	}
}

//---------------------------------------------------------------------------
// Accessors
//---------------------------------------------------------------------------

string SQLCommand :: TableName() const {
	return mTable;
}

string SQLCommand :: Separator() const {
	return mSep;
}

bool SQLCommand :: HaveDataNames() const {
	return mDataCols.size() > 0 && mDataCols[0].mColName != "";
}

const SQLColSpec::Vec & SQLCommand :: DataCols() const {
	return mDataCols;
}

const SQLColSpec::Vec & SQLCommand :: WhereCols() const {
	return mWhereCols;
}

//---------------------------------------------------------------------------
// Validation
//---------------------------------------------------------------------------

void SQLCommand :: MustHaveDataNames() const {
	if ( mDataCols.size() == 0 || mDataCols[0].mColName == "" ) {
		CSVTHROW( "Need column names specified by " << FLAG_COLS << " flag" );
	}
}

void SQLCommand ::  MustHaveWhereNames() const {
	if ( mWhereCols.size() == 0 || mWhereCols[0].mColName == "" ) {
		CSVTHROW( "Need column names specified by " << FLAG_WHERE << " flag" );
	}
}

//---------------------------------------------------------------------------
// Build the WHERE clause from the column specs. Note that referenced
// CSV record fields must exist, unlike most other csvfix commands.
//---------------------------------------------------------------------------

string SQLCommand :: MakeWhereClause( const CSVRow & row  ) const {

	string wc;

	for ( unsigned int i = 0; i < WhereCols().size(); i++ ) {

		unsigned int wi = WhereCols().at( i ).mField;
		if ( wi >= row.size()  ) {
			CSVTHROW( "Required field " << wi + 1 << " missing in input" );
		}

		if ( wc != "" ) {
			wc += " AND ";
		}

		string field = EmptyToNull( row[wi] );

		wc +=  WhereCols().at(i).mColName
				+ (field == "NULL"  ? " IS " : " = " )
				+ ( (DoSQLQuote( i ) && ! NoNullQuote( field ))
					? ALib::SQuote( ALib::SQLQuote( field ) )
					: field );
	}

	return "WHERE " + wc;
}

//---------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

SQLInsertCommand ::	SQLInsertCommand( const string & name,
										const string & desc )
		: SQLCommand( name, desc, INS_HELP ), mHaveColNames( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, true, 1 ) );
}

//---------------------------------------------------------------------------
// Read all inputs and transform into SQL INSERT statements. A table name
// must be specified. Now changed so that column names must be specified too.
// This has been done to make the generated SQL safer.
//---------------------------------------------------------------------------

int SQLInsertCommand ::	Execute( ALib::CommandLine & cmd ) {

	GetCommonValues( cmd );
	BuildDataCols( cmd );
	MustHaveDataNames();	// need col names too

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( ! Skip( row ) ) {
			io.Out() << CreateInsertSQL( TableName(), row );
			io.Out() << Separator();
		}
	}

	return 0;
}

//---------------------------------------------------------------------------
// If column names were specified, create a comma-separated list of them.
//---------------------------------------------------------------------------

string SQLInsertCommand :: CreateColNames() const {

	if ( ! HaveDataNames() ) {
		return "";
	}

	string cols = "( ";
	for ( unsigned int i = 0; i < DataCols().size(); i++ ) {
		if ( i != 0 ) {
			cols += ", ";
		}
		cols += DataCols().at(i).mColName;
	}
	cols += " )";

	return cols;
}

//---------------------------------------------------------------------------
// Create value list for INSERT.  Note that referenced fields must exist.
//---------------------------------------------------------------------------

string SQLInsertCommand :: CreateValues( const CSVRow & row ) const {

	string vals = "";

	for ( unsigned int i = 0; i < DataCols().size(); i++ ) {
		unsigned int fi = DataCols().at(i).mField;
		if ( fi >= row.size() ) {
			CSVTHROW( "Required field " << fi + 1 << " missing from input" );
		}
		if ( vals != "" ) {
			vals += ", ";
		}

		string field = EmptyToNull( row[fi] );
		vals += ( DoSQLQuote( i ) && ! NoNullQuote( field ) )
					? ALib::SQuote( ALib::SQLQuote( field ) )
					: field;
	}

	return " VALUES( " + vals + ")";
}

//---------------------------------------------------------------------------
// See if there is a column spec for this column index
//---------------------------------------------------------------------------

bool SQLInsertCommand :: HaveColSpec( unsigned int idx ) const {
	if ( DataCols().size() == 0 ) {
		return true;
	}
	else {
		for ( unsigned int i = 0; i < DataCols().size(); i++ ) {
			if ( DataCols().at(i).mField == idx ) {
				return true;
			}
		}
		return false;
	}
}

//---------------------------------------------------------------------------
// Build the INSERT statement from its component parts.
//---------------------------------------------------------------------------

string SQLInsertCommand :: CreateInsertSQL( const string & table,
											const CSVRow & row ) {
	string sql = "INSERT INTO " + table + " ";
	sql += CreateColNames();
	sql += CreateValues( row );
	return sql;
}


//---------------------------------------------------------------------------
// Update command is similar to insert, but will have a WHERE clause - we
// don't support UPDATE without WHERE, for safety reasons.
//---------------------------------------------------------------------------

SQLUpdateCommand ::	SQLUpdateCommand( const string & name,
										const string & desc )
		: SQLCommand( name, desc, UPD_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_WHERE, true, 1 ) );
}

//---------------------------------------------------------------------------
// Use flags to set up data (use for SET clause) and where columns, then
// transgorm input to UPDATE statements.
//---------------------------------------------------------------------------

int SQLUpdateCommand ::	Execute( ALib::CommandLine & cmd ) {


	GetCommonValues( cmd );
	BuildDataCols( cmd );
	BuildWhereCols( cmd );
	MustHaveDataNames();
	MustHaveWhereNames();

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( ! Skip( row ) ) {
			io.Out() << CreateUpdateSQL( TableName(), row );
			io.Out() << Separator();
		}
	}

	return 0;
}

//---------------------------------------------------------------------------
// Create the UPDATE statement.
//---------------------------------------------------------------------------

string SQLUpdateCommand ::	CreateUpdateSQL( const string & table,
												const CSVRow & row ) {

	string sql = "UPDATE "
					+ table
					+ " "
					+ MakeSetClause( row )
					+ " "
					+ MakeWhereClause( row );
	return sql;
}

//---------------------------------------------------------------------------
// Create SET clause which looks like:
//
//	  SET foo = '1', bar = '2'
//
// The clause is created from the base class data columns. Referenced columns
// must exist n the CSV input row.
//---------------------------------------------------------------------------

string SQLUpdateCommand :: MakeSetClause( const CSVRow & row ) const {

	string sc;

	for ( unsigned int i = 0; i < DataCols().size(); i++ ) {

		unsigned int fi = DataCols().at( i ).mField;
		if ( fi >= row.size() ) {
			CSVTHROW( "Required field " << fi + 1 << " missing from input" );
		}

		if ( sc != "" ) {
			sc += ", ";
		}

		string field = EmptyToNull( row[fi] );

		sc +=  DataCols().at(i).mColName
				+ " = "
				+ (( DoSQLQuote( i ) && ! NoNullQuote( field ) )
					? ALib::SQuote( ALib::SQLQuote( field ) )
					: field );

	}

	return "SET " + sc;
}

//------------------------------------------------------------------------
// Delete command is like an update without any data
//---------------------------------------------------------------------------

SQLDeleteCommand :: SQLDeleteCommand( const string & name,
										const string & desc )
	: SQLCommand( name, desc, DEL_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_WHERE, true, 1 ) );
}

//---------------------------------------------------------------------------
// DELETE statements only have WHERE clause to check
//---------------------------------------------------------------------------

int SQLDeleteCommand :: Execute( ALib::CommandLine & cmd ) {

	GetCommonValues( cmd );
	BuildWhereCols( cmd );
	MustHaveWhereNames();

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( ! Skip( row ) ) {
			io.Out() << CreateDeleteSQL( TableName(), row );
			io.Out() << Separator();
		}
	}

	return 0;
}

//---------------------------------------------------------------------------
// Build the DELETE statement text
//---------------------------------------------------------------------------

string SQLDeleteCommand :: CreateDeleteSQL( const string & table,
												const CSVRow & row ) {
	string wc = MakeWhereClause( row );
	string dc = "DELETE FROM " + table + " " + wc;
	return dc;
}


} // namespace

// end

