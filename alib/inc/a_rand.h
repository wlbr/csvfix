//---------------------------------------------------------------------------
// a_rand.h
//
// random number generation for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_RAND_H
#define INC_A_RAND_H

#include "a_base.h"

namespace ALib {

//---------------------------------------------------------------------------
// Random number generator class
//---------------------------------------------------------------------------

class RandGen {

	public:

		RandGen( long seed = 0 );

		void SetSeed( long seed );
		void Randomise();

		int NextInt();
		int NextInt( int begin, int end );

		double NextReal();

	private:

		unsigned long mSeed;

};

//---------------------------------------------------------------------------
// Stand alone functions use RandGen class
//---------------------------------------------------------------------------

int Random();
int Random( int begin, int end );
void Randomise( long n );
void Randomise();

//---------------------------------------------------------------------------
// End of ALib stuff
//---------------------------------------------------------------------------

}	// namespace

#endif

