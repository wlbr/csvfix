//---------------------------------------------------------------------------
// csved_util.cpp
//
// utilities for CSVED
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_env.h"
#include "a_str.h"
#include "a_collect.h"
#include "csved_util.h"
#include "csved_except.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Convert comma list to vector of ints to be used as col index. We now allow
// ranges in the form n1:n2.
//---------------------------------------------------------------------------

static void MakeAscending( int n1, int n2, FieldList & fl ) {
	while( n1 <= n2 ) {
		fl.push_back( n1 - 1 );
		n1++;
	}
}

static void MakeDescending( int n1, int n2, FieldList & fl ) {
	while( n1 >= n2 ) {
		fl.push_back( n1 - 1 );
		n1--;
	}
}

void CommaListToIndex( const ALib::CommaList & cl,
						FieldList & idx ) {
	idx.clear();
	for ( unsigned int i = 0; i < cl.Size(); i++ ) {
		string cle = cl.At(i);
		vector <string> fl;
		ALib::Split( cle, ':', fl );
		if ( fl.size() > 2 ) {
			CSVTHROW( "Invalid field: " << cle );
		}
		else if ( fl.size() == 2 ) {
			if ( ! (ALib::IsInteger(fl[0]) && ALib::IsInteger(fl[1])) ) {
				CSVTHROW( "Invalid range: " << cle );
			}
			int n1 = ALib::ToInteger( fl[0] );
			int n2 = ALib::ToInteger( fl[1] );
			if ( n1 < 1 || n2 < 1 ) {
				CSVTHROW( "Invalid range: " << cle );
			}
			if ( n1 < n2 ) {
				MakeAscending( n1, n2, idx );
			}
			else {
				MakeDescending( n1, n2, idx );
			}
		}
		else {
			if ( ! ALib::IsInteger( cle ) ) {
				CSVTHROW( "Need integer, not '" << cle << "'" );
			}
			int n = ALib::ToInteger( cle );
			if ( n < 1 ) {
				CSVTHROW( "Index must be greater than zero, not '" << cle << "'" );
			}
			idx.push_back( n - 1 );		// convert to zero-based
		}
	}
}


//---------------------------------------------------------------------------
// Compare two CSV rows - return as for strcmp. If field list is provided,
// compare only fields in list.
//----------------------------------------------------------------------------

int CmpRow( const CSVRow & a, const CSVRow & b, const FieldList & f ) {
	unsigned int n = std::max( a.size(), b.size() );
	for ( unsigned int i = 0; i < n; i++ ) {

		if ( f.size() && ! ALib::Contains( f, i ) ) {
			continue;
		}

		string fa = GetField( a, i );
		string fb = GetField( b, i );

		if ( fa == fb ) {
			continue;
		}
		else if ( fa < fb ) {
			return -1;
		}
		else {
			return 1;
		}
	}
	return 0;
}

//----------------------------------------------------------------------------
// Get field or empty string if field does not exist
//----------------------------------------------------------------------------

std:: string GetField( const CSVRow & row, unsigned int  i ) {
	return i >= row.size() ? "" : row[i];
}

//----------------------------------------------------------------------------
// Check that not both of two flags are specified, and throw if they are.
//----------------------------------------------------------------------------

void NotBoth( const ALib::CommandLine & cmd, const std::string & a,
				const std::string & b, ReqOp  r  ) {

	if ( cmd.HasFlag( a ) && cmd.HasFlag( b ) ) {
		CSVTHROW( "Cannot specify both " << a << " and " << b << " options");
	}

	if ( r == ReqOp::Required  && ! ( cmd.HasFlag( a ) || cmd.HasFlag(b) ) ) {
		CSVTHROW( "Need one of  " << a << " or " << b  << " options" );
	}
}


//---------------------------------------------------------------------------

}	// end namespace

// end


