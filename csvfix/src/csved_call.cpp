//---------------------------------------------------------------------------
// csved_call.cpp
//
// call function in dll from csvfix
// this is windows specific code
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_call.h"
#include "csved_evalvars.h"
#include "csved_strings.h"
#include <string>

#include <windows.h>

using std::string;

//----------------------------------------------------------------------------

namespace CSVED {

//----------------------------------------------------------------------------
// Default size of buffer used to communicate with DLL
//----------------------------------------------------------------------------

const unsigned int DEF_OUTBUF_SIZE = 4096;

//---------------------------------------------------------------------------
// Register call  command
//---------------------------------------------------------------------------

static RegisterCommand <CallCommand> rc1_(
	CMD_CALL,
	"call function in DLL"
);

//----------------------------------------------------------------------------
// Help
//----------------------------------------------------------------------------

const char * const CALL_HELP = {
	"call function in DLL\n"
	"usage: csvfix call  [flags] [file ...]\n"
	"where flags are:\n"
	"  -fnc name\tname of function to call\n"
	"  -dll name\tfilename of DLL containing function\n"
	"  -f fields\tindexes of fields to pass to the function\n"
	"  -bs size\tsize in Kbytes of buffer used to communicate with DLL (default 4K)\n "
	"#ALL,SKIP,PASS"
};

//----------------------------------------------------------------------------
// The call  command
//----------------------------------------------------------------------------

CallCommand :: CallCommand( const string & name, const string & desc )
				: Command( name, desc, CALL_HELP ),
					mOutBufSize( DEF_OUTBUF_SIZE ) {

	AddFlag( ALib::CommandLineFlag( FLAG_DLL, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FUNC, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_BSIZE, false, 1 ) );
}

//----------------------------------------------------------------------------
// Load function from DLL and then call it on fields. A non-zero return value
// from function of means either skip this row on output ( for
// negative return values), or report an error (positive values).
//
// ??? Need to thnk more about return value
//----------------------------------------------------------------------------

int CallCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );
	std::vector <char> outbuf( mOutBufSize );

	HMODULE dll = LoadLibrary( mDLL.c_str() );
	if ( dll == NULL ) {
		CSVTHROW( "LoadLibrary call on " << mDLL << " failed" );
	}
	mFunc = (FuncType) GetProcAddress( dll, mFuncName.c_str() );
	if ( mFunc == NULL ) {
		CSVTHROW( "Cannot load function " << mFuncName << "from DLL " << mDLL );
	}

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {

		if ( ! Skip( row ) ) {
			continue;
		}
		if ( Pass( row ) ) {
			io.WriteRow( row );
			continue;
		}

		int rv = CallOnFields( row, &outbuf[0] );
		if ( rv == 0 ) {
			io.WriteRow( row );
		}
		else if ( rv > 0 ) {
			CSVTHROW( mFuncName << " returned error code " << rv );
		}
		else {
			// do nothing - negative values just mean skip output
		}
	}
	return 0;
}

//----------------------------------------------------------------------------
// Helper to extract single field  from output data
//----------------------------------------------------------------------------

static string ExtractField( const char * s, int & pos ) {
	string field;
	while( s[pos] ) {
		field += s[pos++];
	}
	return field;
}

//----------------------------------------------------------------------------
// Call DLL function on specified fields. If none are specified, call it on
// all fields. Fields returned from DLL are appended to the existing data on
// output, provided the function returned zero.
//----------------------------------------------------------------------------

int CallCommand :: CallOnFields( CSVRow & row, char * buffer ) {
	string fields;
	int fc = 0;
	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( mFields.size() == 0 || ALib::Contains( mFields, i ) ) {
			fields += row[i];
			fields += '\0';
			fc++;
		}
	}
	int ofc = 0;
	int rv = mFunc( fc, fields.c_str(), &ofc, buffer, mOutBufSize );
	if ( rv == 0  ) {
		int pos = 0;
		while( ofc-- ) {
			string field = ExtractField( buffer, pos );
			pos++;
			row.push_back( field );
		}
	}
	return rv;
}


//----------------------------------------------------------------------------
// Handle command line options
//----------------------------------------------------------------------------

void CallCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	string bs = cmd.GetValue( FLAG_BSIZE, ALib::Str( DEF_OUTBUF_SIZE ) );
	if ( ! ALib::IsInteger( bs )) {
		CSVTHROW( "Value for buffer size must be integer" );
	}
	mOutBufSize = ALib::ToInteger( bs ) * 1024;
	if ( mOutBufSize < DEF_OUTBUF_SIZE ) {
		CSVTHROW( "Output buffer size too small" );
	}
	mDLL = cmd.GetValue( FLAG_DLL );
	mFuncName = cmd.GetValue( FLAG_FUNC );
	ALib::CommaList cl( cmd.GetValue( FLAG_COLS ) );
	CommaListToIndex( cl, mFields );

}


//----------------------------------------------------------------------------


} // namespace

