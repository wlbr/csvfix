//---------------------------------------------------------------------------
// a_enc.cpp
//
// encode/decode functions
//
// This implementation uses the base64 encoding code from the Apache Web
// Seerver. The copyright of this code is:
//
//		Copyright 2000-2005 The Apache Software Foundation or its 
//		licensors, as applicable.
//
// The remainder of the code is covered by the normal ALib copyright:
//
//		Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include <iostream>
#include "a_base.h"
#include "a_enc.h"
#include "a_except.h"

using std::string;
using std::vector;

//---------------------------------------------------------------------------

namespace ALib {	


//---------------------------------------------------------------------------
// Ascii character table for base64
//---------------------------------------------------------------------------

static const unsigned char pr2six[256] =
{
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

//---------------------------------------------------------------------------
// Apache function to get length of the buffer needed to hold the
// decoded version of the encoded string bufcoded.
//---------------------------------------------------------------------------

static int apr_base64_decode_len(const char *bufcoded)
{
    int nbytesdecoded;
    register const unsigned char *bufin;
    register int nprbytes;

    bufin = (const unsigned char *) bufcoded;
    while (pr2six[*(bufin++)] <= 63);

    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    return nbytesdecoded + 1;
}


//---------------------------------------------------------------------------
// Apache code to decode sequence of binary bytes
//---------------------------------------------------------------------------

static int apr_base64_decode( unsigned char *bufplain,
								 const char *bufcoded)
{
    int nbytesdecoded;
    register const unsigned char *bufin;
    register unsigned char *bufout;
    register int nprbytes;

    bufin = (const unsigned char *) bufcoded;
    while (pr2six[*(bufin++)] <= 63);
    nprbytes = (bufin - (const unsigned char *) bufcoded) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    bufout = (unsigned char *) bufplain;
    bufin = (const unsigned char *) bufcoded;

    while (nprbytes > 4) {
	*(bufout++) =
	    (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
	*(bufout++) =
	    (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
	*(bufout++) =
	    (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
	bufin += 4;
	nprbytes -= 4;
    }

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) {
	*(bufout++) =
	    (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
    }
    if (nprbytes > 2) {
	*(bufout++) =
	    (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
    }
    if (nprbytes > 3) {
	*(bufout++) =
	    (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
    }

    nbytesdecoded -= (4 - nprbytes) & 3;
    return nbytesdecoded;
}

//---------------------------------------------------------------------------
// The base64 digit set
//---------------------------------------------------------------------------

static const char basis_64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//---------------------------------------------------------------------------
// Apache code - given length of un-encoded string, get legth of encoding
//---------------------------------------------------------------------------
 
int apr_base64_encode_len(int len)
{
    return ((len + 2) / 3 * 4) + 1;
}


//---------------------------------------------------------------------------
// Apache code - Do base64 encoding with null termination
//---------------------------------------------------------------------------

int apr_base64_encode( char *encoded,
						const unsigned char *string, int len)
{
    int i;
    char *p;

    p = encoded;
    for (i = 0; i < len - 2; i += 3) {
	*p++ = basis_64[(string[i] >> 2) & 0x3F];
	*p++ = basis_64[((string[i] & 0x3) << 4) |
	                ((int) (string[i + 1] & 0xF0) >> 4)];
	*p++ = basis_64[((string[i + 1] & 0xF) << 2) |
	                ((int) (string[i + 2] & 0xC0) >> 6)];
	*p++ = basis_64[string[i + 2] & 0x3F];
    }
    if (i < len) {
	*p++ = basis_64[(string[i] >> 2) & 0x3F];
	if (i == (len - 1)) {
	    *p++ = basis_64[((string[i] & 0x3) << 4)];
	    *p++ = '=';
	}
	else {
	    *p++ = basis_64[((string[i] & 0x3) << 4) |
	                    ((int) (string[i + 1] & 0xF0) >> 4)];
	    *p++ = basis_64[((string[i + 1] & 0xF) << 2)];
	}
	*p++ = '=';
    }

    *p++ = '\0';
    return p - encoded;
}

//---------------------------------------------------------------------------
// Encode string, returning encoding
//---------------------------------------------------------------------------

string Base64Encode( const string & s ) {
	int blen = apr_base64_encode_len( s.size() );
	vector <char> buf( blen + 1);
	apr_base64_encode( &buf[0], (const unsigned char *) s.c_str(), s.size() );
	return &buf[0];		
}

//---------------------------------------------------------------------------
// Decode string, returning decoding
//---------------------------------------------------------------------------

string Base64Decode( const string & s ) {
	int blen = apr_base64_decode_len( s.c_str() );
	vector <char> buf( blen + 1 );
	apr_base64_decode( (unsigned char *) &buf[0], s.c_str() );
	buf[blen] = 0;
	return string(&buf[0], blen );
}

//---------------------------------------------------------------------------
// Helper to convert low 4 bits to single hex digit
//---------------------------------------------------------------------------

static char NibbleToHex( unsigned char c ) {
	unsigned int n = c & 0x0f;
	if ( n < 10 ) {
		return '0' + n;
	}
	else {
		return 'A' + (n - 10);
	}
}

//---------------------------------------------------------------------------
// Helper to convert hex digit to number in range 0 - 15
//---------------------------------------------------------------------------

static unsigned int HexToNibble( unsigned char hd )  {
	if ( hd <= '9' ) {
		return hd - '0';
	}
	else if ( hd >= 'A' && hd <= 'F' ) {
		return 10 + (hd - 'A');
	}
	else if ( hd >= 'a' && hd <= 'f' ) {
		return 10 + (hd - 'a');
	}
	else {
		ATHROW( "Invalid hex digit '" << hd << "'" );
	} 
}

//---------------------------------------------------------------------------
// Convert single character to 2-digit hex number
//---------------------------------------------------------------------------

string CharToHex( unsigned char c ) {
	char s[3];
	s[0] = NibbleToHex( (c >> 4) & 0x0f );
	s[1] = NibbleToHex( c & 0x0f);
	s[2] = 0;
	return s;
}

//---------------------------------------------------------------------------
// Convert string of characters to hex string
//---------------------------------------------------------------------------

string StrToHex( const string & s ) {
	string h;
	unsigned int n = s.size();
	for ( unsigned int i = 0; i < n; i++ ) {
		h += CharToHex( s[i] );
	}
	return h;
}

//---------------------------------------------------------------------------
// Convert 2 hex digits at position 'pos' to numeric value in range 0 - 255
//---------------------------------------------------------------------------
	
unsigned char HexToChar( const string & s, unsigned int pos ) {
	unsigned int n1 = HexToNibble( s[pos] );
	unsigned int n2 = HexToNibble( s[pos + 1] );
	return n1 * 16 + n2;			
}

//---------------------------------------------------------------------------
// Convert string of hex digits back to character string
//---------------------------------------------------------------------------

string HexToStr( const string & h ) {
	string s;
	unsigned int n = h.size();
	for ( unsigned int i = 0; i < n; i += 2 ) {
		s += HexToChar( h, i );
	}
	return s;
}

//---------------------------------------------------------------------------

}	// end namespace

// end
