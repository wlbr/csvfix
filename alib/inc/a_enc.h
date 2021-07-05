//---------------------------------------------------------------------------
// a_enc.h
//
// encode/decode functions
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_ENC_H
#define INC_A_ENC_H

#include "a_base.h"

namespace ALib {

//---------------------------------------------------------------------------
// Hex encode/decode functions
//---------------------------------------------------------------------------

std::string CharToHex( unsigned char c );
unsigned char HexToChar( const std::string & s, unsigned int pos );
std::string StrToHex( const std::string & s );
std::string HexToStr( const std::string & h );

//---------------------------------------------------------------------------
// Base64 encode & decode
//---------------------------------------------------------------------------

std::string Base64Encode( const std::string & s );
std::string Base64Decode( const std::string & s );

}	// end namespace

#endif

