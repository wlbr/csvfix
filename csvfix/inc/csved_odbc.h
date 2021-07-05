//---------------------------------------------------------------------------
// csved_odbc.h
//
// odbc database stuff for csvfix
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_ODBC_H
#define INC_CSVED_ODBC_H

#include "a_base.h"
#include "a_db.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------
// Base command handles connectivity
//----------------------------------------------------------------------------

class ODBCGetCommand : public Command {

	public:

		ODBCGetCommand( const std::string & name,
						const std::string & desc );
		~ODBCGetCommand();

	private:

		void Connect();
		void Exec( const std::string & sql );
		ALib::DbStatement * Stmt();
		void ProcessFlags( const ALib::CommandLine & cmd );
		int Execute( ALib::CommandLine & cmd );

		ALib::DbConnection mConnection;
		ALib::DbStatement * mStatement;

		std::string mConnStr;
		std::string mSql, mNull;
		bool mUseColNames;
};


//------------------------------------------------------------------------

}	// end namespace

#endif

