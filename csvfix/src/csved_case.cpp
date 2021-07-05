//---------------------------------------------------------------------------
// csved_case.cpp
//
// Character case conversion for csvfix
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_collect.h"
#include "csved_cli.h"
#include "csved_case.h"
#include "csved_except.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register case conversion commands
//---------------------------------------------------------------------------

static RegisterCommand <UpperCommand> rc1_(
	CMD_UPPER,
	"convert to uppercase"
);

static RegisterCommand <LowerCommand> rc2_(
	CMD_LOWER,
	"convert to lowercase"
);

static RegisterCommand <MixedCommand> rc3_(
	CMD_MIXED,
	"convert to mixed case"
 );

//---------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

CaseBase :: CaseBase( const std::string & name,
					const std::string & desc )
		: Command( name, desc ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );

}

//---------------------------------------------------------------------------
// Change case
//---------------------------------------------------------------------------

int CaseBase :: Execute( ALib::CommandLine & cmd ) {

	ALib::CommaList cols( cmd.GetValue( FLAG_COLS ) );
	std::vector <unsigned int> colindex;
	CommaListToIndex( cols, colindex );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		ProcessRow( row, colindex );
		io.WriteRow( row );
	}

	return 0;
}

//---------------------------------------------------------------------------
// Convert all columns (or those specified by fields flag) depending on
// class type via the virtual Transform() function (defined inline in header)
//---------------------------------------------------------------------------

void CaseBase :: ProcessRow( CSVRow & row, vector <unsigned int > & ci ) {
	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( ci.size() == 0 || ALib::Contains( ci, i ) ) {
			Transform( row[i] );
		}
	}
}

//----------------------------------------------------------------------------
// Provide help - Explain() provides command specific stuff.
//----------------------------------------------------------------------------

string CaseBase :: Help() const {
	string s = Explain();
	s +=
		"where flags are:\n"
		"  -f fields\tlist of fields to convert (default is all)\n"
		"  -ibl\t\tignore blank input lines\n"
		"  -ifn\t\tignore field name record\n"
		"  -smq\t\tuse smart quotes on output\n"
		"  -sep sep\tspecify CSV field separator character\n"
		"  -o file\twrite output to file rather than standard output\n";
	return s;
}

//----------------------------------------------------------------------------

}	// end namespace

// end
