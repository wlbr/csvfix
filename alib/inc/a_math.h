//------------------------------------------------------------------------
// a_math.h
//
// math  stuff for alib
//
// Copyright (C) 2009 Neil Butterworth
//------------------------------------------------------------------------

#ifndef INC_A_MATH_H
#define INC_A_MATH_H

namespace ALib {

//----------------------------------------------------------------------------
// Round number up
//----------------------------------------------------------------------------

double Round( double d );

//----------------------------------------------------------------------------
// Is value in range? works for anything that supports relational ops, but
// is intended promarily for numerics, hence we don't use refs for params.
//----------------------------------------------------------------------------

template <typename VTYPE, typename BTYPE, typename ETYPE>
bool InRange( VTYPE v, BTYPE b, ETYPE e ) {
	return v >= b && v <= e;
}

//----------------------------------------------------------------------------

}	// namespace

#endif

