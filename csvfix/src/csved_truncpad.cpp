//---------------------------------------------------------------------------
// csved_truncpad.cpp
//
// truncation and padding for csvfix
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "csved_cli.h"
#include "csved_except.h"
#include "csved_truncpad.h"
#include "csved_strings.h"

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register commands
//----------------------------------------------------------------------------

static RegisterCommand <TruncCommand> rc1_(
	CMD_TRUNC,
	"truncate CSV records"
);

static RegisterCommand <PadCommand> rc2_(
	CMD_PAD,
	"pad CSV records to fixed number of fields"
);


//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const TRUNC_HELP = {
	"truncate CSV records by removing rightmost fields\n"
	"usage: csvfix truncate [flags] [file ...]\n"
	"where flags are:\n"
	"  -n count\tnumber of fields to truncate to\n"
	"#ALL,SKIP,PASS"
};

const char * const PAD_HELP = {
	"pad CSV output by adding fields to right-hand side of record\n"
	"usage: csvfix pad [flags] [file ...]\n"
	"where flags are:\n"
	"  -n count\tnumber of fields to pad to\n"
	"  -p vals\tvalues  to use for paddding (default is empty string)\n"
	"#ALL,SKIP,PASS"
};

//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

TruncPadBase :: TruncPadBase( const string & name,
								const string & desc,
								const string & help )
	: Command( name, desc, help ) {

	AddFlag( ALib::CommandLineFlag( FLAG_NUM, true, 1 ) );

}

PadCommand :: PadCommand( const string & name,
							const string & desc )
			: TruncPadBase( name, desc, PAD_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_PAD, false, 1 ) );
}


TruncCommand :: TruncCommand( const string & name,
								const string & desc )
			: TruncPadBase( name, desc, TRUNC_HELP ) {
}

//---------------------------------------------------------------------------
// Perform truncation on row. CommaList param not used.
//---------------------------------------------------------------------------

void TruncCommand :: ProcessRow( CSVRow & row, unsigned int ncols,
									const ALib::CommaList &  ) {
	if ( ncols < row.size() ) {
		CSVRow nrow( ncols );
		std::copy( row.begin(), row.begin() + ncols, nrow.begin() );
		row.swap( nrow );
	}
}

//---------------------------------------------------------------------------
// Perform padding. Use values in comma list to pad.
//---------------------------------------------------------------------------

void PadCommand :: ProcessRow( CSVRow & row, unsigned int ncols,
									const ALib::CommaList & cl  ) {
	unsigned int sz = row.size();
	for ( unsigned int i = sz; i < ncols; i++ ) {
		if ( cl.Size() == 0 ) {
			row.push_back( "" );
		}
		else {
			unsigned int ci = i - sz;
			if ( ci >= cl.Size() ) {
				row.push_back( cl.At( cl.Size() - 1 ) );
			}
			else {
				row.push_back( cl.At( ci ) );
			}
		}
	}
}

//---------------------------------------------------------------------------
// Do truncation or padding depending on command type.
//---------------------------------------------------------------------------

int TruncPadBase :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	string ps = cmd.GetValue( FLAG_PAD );
	ALib::CommaList padding( ps );
	unsigned int ncols = padding.Size();

	bool ncolspec = false;	// use explicit size or not

	if ( ncols == 0 || cmd.HasFlag( FLAG_NUM ) ) {
		if ( ! cmd.HasFlag( FLAG_NUM ) ) {
			CSVTHROW( "Need -n flag to specify field count" );
		}
		ncolspec = true;
		string nv = cmd.GetValue( FLAG_NUM );
		if ( ALib::ToInteger( nv, "-n flag needs integer value" ) < 0 ) {
			CSVTHROW( FLAG_NUM << " needs value greater or equal to zero" );
		}
		ncols = ALib::ToInteger( nv );
	}

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {

		if ( Skip( row ) ) {
			continue;
		}

		if ( ! Pass( row ) ) {
			unsigned int nc = ncolspec ? ncols : row.size() + padding.Size();
			ProcessRow( row, nc, padding );
		}

		io.WriteRow( row );
	}

	return 0;
}

//---------------------------------------------------------------------------

}	// end namespace

// end
