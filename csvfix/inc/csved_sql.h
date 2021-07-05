//---------------------------------------------------------------------------
// csved_sql.h
//
// generate SQL statements from CSV
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_SQL_H
#define INC_CSVED_SQL_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------
// Spexify a column as 1-based index and name
//----------------------------------------------------------------------------

struct SQLColSpec {

	typedef std::vector <SQLColSpec> Vec;

	unsigned int mField;
	std::string mColName;

	SQLColSpec( unsigned int f, const std::string & c )
				: mField( f ), mColName( c ) {}

	friend std::ostream & operator << ( std::ostream & os,
										const SQLColSpec & s ) {
		return os << s.mField << ":" << s.mColName;
	}
};

//---------------------------------------------------------------------------
// Base class for SQL commands specifies columns, table etc.
//----------------------------------------------------------------------------

class SQLCommand : public Command {

	public:

		SQLCommand( const std::string & name,
						const std::string & desc,
						const std::string & help );
	protected:

		void GetCommonValues( ALib::CommandLine & cmd );

		void BuildDataCols( const ALib::CommandLine & cmd );
		void BuildWhereCols( const ALib::CommandLine & cmd );

		void MustHaveDataNames() const;
		void MustHaveWhereNames() const;

		std::string TableName() const;
		std::string Separator() const;
		bool HaveDataNames() const;

		const SQLColSpec::Vec & DataCols() const;
		const SQLColSpec::Vec & WhereCols() const;

		std::string MakeWhereClause( const CSVRow & row ) const;

		bool DoSQLQuote( unsigned int i ) const;
		bool NoNullQuote( const std::string & ns ) const;
		std::string EmptyToNull( const std::string & f ) const;

	private:

		void BuildCols( const ALib::CommandLine & cmd,
						const std::string & flag,
						SQLColSpec::Vec & cols );

		std::string mTable, mSep;
		FieldList mNoQuote;
		SQLColSpec::Vec mDataCols, mWhereCols;
		bool mQuoteNulls, mEmptyNulls;
};

//---------------------------------------------------------------------------
// Generate INSERT statements from CSV
//----------------------------------------------------------------------------

class SQLInsertCommand : public SQLCommand {

	public:

		SQLInsertCommand( const std::string & name,
							const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		std::string CreateInsertSQL( const std::string & table,
									const CSVRow & row );

		bool HaveColSpec( unsigned int idx ) const;

		std::string CreateColNames() const;
		std::string CreateValues( const CSVRow & row ) const;



		bool mHaveColNames;

};

//---------------------------------------------------------------------------
// Generate UPDATE statements from CSV
//----------------------------------------------------------------------------

class SQLUpdateCommand : public SQLCommand {

	public:

		SQLUpdateCommand( const std::string & name,
							const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		std::string CreateUpdateSQL( const std::string & table,
										const CSVRow & row );

		std::string MakeSetClause( const CSVRow & row  ) const;


};

//---------------------------------------------------------------------------
// Generate DELETE statements from CSV
//----------------------------------------------------------------------------

class SQLDeleteCommand : public SQLCommand {

	public:

		SQLDeleteCommand( const std::string & name,
							const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		std::string CreateDeleteSQL( const std::string & table,
										const CSVRow & row );

};

//------------------------------------------------------------------------

}	// end namespace

#endif

