//---------------------------------------------------------------------------
// csved_call.h
//
// call dll function from csvfix
// currently windows-only code
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_CALL_H
#define INC_CSVED_CALL_H

#include "a_base.h"
#include "a_expr.h"
#include "csved_command.h"

namespace CSVED {


//----------------------------------------------------------------------------
// Function in DLL needs to look like this. CSVfix passes the number
// of fields to the DLL as 'ifc' and the list of fields as 'input'. Each
// field in 'input' is terminated by a null byte. The DLL must not change or free
// this data. CSVfix also passes the maximum size of the output buffer
// as 'outsize' - the DLL must not write beyound this size.
//
// The DLL function must return fields in 'output', each field being terminated
// with a null-byte. The number of fieds must be returned in 'ofc'. Neither
// 'ofc' or 'output' must be freed by the DLL.
//
// The function must return zero on success. A negitive return value means
// that any returned fields must not be appended to the existing row. A positive
// return value indicates a fatal error, in which case CSVFix will terminare
// with an error message.
//----------------------------------------------------------------------------

extern "C" {
	typedef int (*FuncType)
		( int ifc, 					// number of fields in input
		  const char * input, 		// each field must end with \0
		  int * ofc, 				// DLL must set to number of output fields
		  char * output, 			// fixed-sized output data
		  int outsize  );			// fixed size of output data
}

//----------------------------------------------------------------------------
// Call command
//----------------------------------------------------------------------------

class CallCommand : public Command {

	public:

		CallCommand( const std::string & name,
					const std::string & desc );

		int Execute( ALib::CommandLine & cmd );


	private:

		void ProcessFlags( const ALib::CommandLine & cmd );
		int CallOnFields( CSVRow & row, char * buffer );

		std::string mDLL, mFuncName;
		FieldList mFields;
		FuncType mFunc;

		unsigned int mOutBufSize;
};

//----------------------------------------------------------------------------

} // namespace

#endif
