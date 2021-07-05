//---------------------------------------------------------------------------
// csved_util.h
//
// utilities for CSVED
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------


#ifndef INC_CSVED_UTIL_H
#define INC_CSVED_UTIL_H

#include "a_base.h"
#include "a_csv.h"
#include "a_env.h"
#include "a_str.h"
#include "a_env.h"

#include "csved_types.h"
#include "csved_except.h"

namespace CSVED {

//---------------------------------------------------------------------------
// Convert comma list to vector of ints to be used as column index
//---------------------------------------------------------------------------

void CommaListToIndex( const ALib::CommaList & cl,
						FieldList & idx );

//---------------------------------------------------------------------------
// Compare two CSV rows - return as for strcmp. If field list is provided,
// compare oonly fields in list.
//----------------------------------------------------------------------------

int CmpRow( const CSVRow & a, const CSVRow & b,
					const FieldList & f = FieldList  () );

//----------------------------------------------------------------------------
// Get field or empty string if field does not exist
//----------------------------------------------------------------------------

std::string GetField( const CSVRow & row, unsigned int  i );


//----------------------------------------------------------------------------
// Check that not both of two flags are specified, and throw if they are.
//----------------------------------------------------------------------------

enum class ReqOp {
	Required, Optional
};

void NotBoth( const ALib::CommandLine & cmd, const std::string & a,
				const std::string & b, ReqOp ro = ReqOp::Optional );


}	// end namespace


#endif

