//---------------------------------------------------------------------------
// a_rand.cpp
//
// random number generation for alib
// algorithm taken from stroustrup 3rd ed
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------


#include <time.h>
#include <assert.h>
#include "a_except.h"
#include "a_rand.h"

//------------------------------------------------------------------------
// begin ALib stuff
//------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// Construct generator with initioal seed
//---------------------------------------------------------------------------

RandGen :: RandGen( long seed )
	: mSeed( seed ) {
}

//---------------------------------------------------------------------------
// Set seed
//---------------------------------------------------------------------------

void RandGen :: SetSeed( long seed ) {
	mSeed = seed;
}

//---------------------------------------------------------------------------
// Seed with current time
//---------------------------------------------------------------------------

void RandGen :: Randomise() {
	SetSeed( time(0) );
}

//---------------------------------------------------------------------------
// Helper to get unsigned value
//---------------------------------------------------------------------------

static int Abs( int n ) { 
	return n & 0x7fffffff; 
}

//---------------------------------------------------------------------------
// Max int size as real
//---------------------------------------------------------------------------

const double RMAX = 2147483648.0;

//---------------------------------------------------------------------------
// Get next integer in sequence
//---------------------------------------------------------------------------

int RandGen :: NextInt() {
	return Abs( mSeed = mSeed * 1103515245 + 12345 ); 
}

//---------------------------------------------------------------------------
// Next integer in the half open range [begin,end)
//---------------------------------------------------------------------------

int RandGen :: NextInt( int begin, int end ) {
	assert( begin < end ); 
	int diff = end - begin;
	int n = int( diff * NextReal() );
	return begin + n;
}

//---------------------------------------------------------------------------
// Get next real in sequence. Values in range 0.0 <= N < 1.0
//---------------------------------------------------------------------------

double RandGen :: NextReal() {
	return Abs( NextInt() ) / RMAX; 
}

//---------------------------------------------------------------------------
// Stand alone functions use common generator - possible thread problems
//---------------------------------------------------------------------------

static RandGen  AGen;		// common generator

int Random() {
	return AGen.NextInt();
}

int Random( int begin, int end ) {
	return AGen.NextInt( begin, end );
}

void Randomise() {
	AGen.Randomise();
}

void Randomise( long n ) {
	AGen.SetSeed( n );
}

//---------------------------------------------------------------------------
// end Alib stuff
//---------------------------------------------------------------------------

}


// end

