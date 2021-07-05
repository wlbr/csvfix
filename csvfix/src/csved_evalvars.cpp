//----------------------------------------------------------------------------
// csved_evalvars.cpp
//
// names for special variables used by the expression evaluator in
// various commands
//
// Copyright (C) 2011 Neil Butterworth
//----------------------------------------------------------------------------

#include "csved_evalvars.h"

namespace CSVED {

void AddVars( ALib::Expression & e, const IOManager & io, const CSVRow & row ) {
	e.ClearPosParams();
	e.AddVar( LINE_VAR, ALib::Str( io.CurrentLine() ));
	e.AddVar( FILE_VAR, ALib::Str( io.CurrentFileName()));
	e.AddVar( FIELD_VAR, ALib::Str( row.size()));
	for ( unsigned int j = 0; j < row.size(); j++ ) {
		e.AddPosParam( row.at( j ) );
	}
}


} // namespace

// end

