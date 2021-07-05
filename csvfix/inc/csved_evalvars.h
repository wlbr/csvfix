//----------------------------------------------------------------------------
// csved_evalvars.h
//
// names for special variables used by the expression evaluator in
// various commands
//
// Copyright (C) 2011 Neil Butterworth
//----------------------------------------------------------------------------

#ifndef INC_CSVED_EVALVARS_H
#define INC_CSVED_EVALVARS_H

#include "a_expr.h"
#include "csved_ioman.h"
#include "csved_types.h"

namespace CSVED {

const char * const LINE_VAR 	= "line";	// var containing current line no
const char * const FILE_VAR 	= "file";	// var containing current file name
const char * const FIELD_VAR 	= "fields"; // var containing CSV field count

// utility function to add variables

void AddVars( ALib::Expression & e, const IOManager & io, const CSVRow & row );


} // namespace

#endif
