//----------------------------------------------------------------------------
// dllmain.cpp
//
// Implementation of a C++ function that can be called via the CSVfix "call"
// command. Copy and modify this code to implement your own function.
//
// Copyright (C) 2012 Neil Butterworth
//----------------------------------------------------------------------------

#include <cstring>
#include <string>
#include <windows.h>

using namespace std;

//----------------------------------------------------------------------------
// Helper function to extract a null-terminated string from the buffer passed
// by the calling CSVfix instance.
//----------------------------------------------------------------------------

string ExtractField( const char * p, int & pos ) {
	string s;
	while( p[pos] ) {
		s += p[pos++];
	}
	return s;
}

//----------------------------------------------------------------------------
// The sample function itself. This example simply places a pair of braces 
// around each field that was passed to it, and returns the result.
//----------------------------------------------------------------------------

extern "C" int  __declspec(dllexport)  MyFunc( 
		int infc, 					// number of fields in input
		const char * in, 			// input fields, each null terminated
		int * outfc, 				// number of fields being returned
		char * out,					// the output fields the function creates 
		int bufsize					//  the output buffer size provided by CSVfix 
	) {

	int pos = 0, n = 0;
	string s;
	while( infc-- ) {				// loop over all input fields
		string field = + "{" + ExtractField( in, pos ) + "}";
		pos++;						// skip null terminator
		s += field;					// add to output string
		s += '\0';					// output fields must be null terminated
		n++;						// count output fields
	}

	// set output field count and copy fields to output buffer
	* outfc = n;			
	memcpy( out, (void*)s.c_str(), s.size() + 1 );
	return 0;
}

//----------------------------------------------------------------------------
// Standard DLLMain function - no need to touch this
//----------------------------------------------------------------------------

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}

// end


