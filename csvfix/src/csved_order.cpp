//---------------------------------------------------------------------------
// csved_order.h
//
// field reordering for CSVfix
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_str.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_except.h"
#include "csved_order.h"
#include "csved_strings.h"
#include <algorithm>

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register order command
//---------------------------------------------------------------------------

static RegisterCommand <OrderCommand> rc1_(
	CMD_ORDER,
	"change field order"
);

//----------------------------------------------------------------------------
// Order command help
//----------------------------------------------------------------------------

const char * const ORDER_HELP = {
	"re-orders, duplicates or removes fields from input\n"
	"usage: csvfix order [flags] [files ...]\n"
	"where flags are:\n"
	"  -f fields\tspecify fields using numeric field indexes\n"
	"  -nc\t\tdo not create fields missing in input\n"
	"  -xf fields\tspecify all but excluded fields\n"
	"  -rf fields\tas for -f, but specify fields from end of record\n"
	"  -fn names\tspecify fields using list of field names\n"
	"\t\tthis reguires that the input contains field name header\n"
	"#SMQ,SEP,IBL,IFN,OFL,SKIP,PASS"

};

//----------------------------------------------------------------------------
// Order command
//----------------------------------------------------------------------------

OrderCommand ::	OrderCommand( const string & name,
								const string & desc )
		: Command( name, desc, ORDER_HELP ),
			mRevOrder( false ), mExclude( false ), mNoCreate( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_EXCLF, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_REVCOLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FNAMES, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_NOCREAT, false, 0 ) );

}

//----------------------------------------------------------------------------
// Get event from IOManager when new input stream is started. Use that to
// find the current field name ordering, if one exists.
//----------------------------------------------------------------------------

void OrderCommand :: OnNewCSVStream( const string &,
										const ALib::CSVStreamParser * p ) {
	if ( mOrderNames.Size() != 0 ) {
		mOrder.clear();
		for ( unsigned int i = 0; i < mOrderNames.Size(); i++ ) {
			string aname = mOrderNames.At( i );
			unsigned int col = p->ColIndexFromName( aname );
			mOrder.push_back( col );
		}

	}
}

//---------------------------------------------------------------------------
// Change ordering for all inputs.
//---------------------------------------------------------------------------

int OrderCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );

	IOManager io( cmd, mOrderNames.Size() != 0 );
	io.AddWatcher( * this );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}

		if ( ! Pass( row ) ) {
			if ( mExclude ) {
				ExcludeFields( row );
			}
			else {
				Reorder( row );
			}
		}
		io.WriteRow( row );
	}
	return 0;
}

//----------------------------------------------------------------------------
// If exclude field flag was used, remove specified fields.
//----------------------------------------------------------------------------

void OrderCommand :: ExcludeFields( CSVRow & row ) {

	CSVRow newrow;

	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( ! ALib::Contains( mOrder, i ) ) {
			newrow.push_back( row[ i ] );
		}
	}

	row.swap( newrow );
}

//---------------------------------------------------------------------------
// Perform actual re-ordering of a row of data using the indexes in mOrder.
// If a field index doesn't exist, output an empty string for that field,
// unless the -nc flag ws set, in which case skip the field.
//---------------------------------------------------------------------------

void OrderCommand :: Reorder( CSVRow & row ) {

	CSVRow newrow;

	if ( mRevOrder ) {
		std::reverse( row.begin(), row.end() );
	}

	for ( unsigned int i = 0; i < mOrder.size(); i++ ) {
		unsigned int ri = mOrder[ i ];
		if ( ri < row.size() ) {
			newrow.push_back( row[ ri ] );
		}
		else {
			if ( ! mNoCreate ) {
				newrow.push_back( "" );
			}
		}
	}

	row.swap( newrow );
}

//----------------------------------------------------------------------------
// Process command line flags.User can specify numeric (one-based)  field
// indexes using the -f flag or field names using -fn. In the latter case,
// the first input record for each file must be a list of field names.
//----------------------------------------------------------------------------

void OrderCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {
	unsigned int ncf = cmd.FlagCount( FLAG_COLS )
						+ cmd.FlagCount( FLAG_REVCOLS )
						+ cmd.FlagCount( FLAG_FNAMES )
						+ cmd.FlagCount( FLAG_EXCLF );
	if ( ncf != 1 ) {
		CSVTHROW( "Need " << FLAG_COLS
							<< " or " << FLAG_FNAMES
							<< " or " << FLAG_REVCOLS
							<< " or " << FLAG_EXCLF
					<< " flags (but only one)" );
	}

	mNoCreate = cmd.HasFlag( FLAG_NOCREAT );

	if ( cmd.HasFlag( FLAG_COLS ) ||  cmd.HasFlag( FLAG_EXCLF )) {
		mExclude = cmd.HasFlag( FLAG_EXCLF );
		ALib::CommaList cl( cmd.GetValue( mExclude ? FLAG_EXCLF : FLAG_COLS ) );
		CommaListToIndex( cl, mOrder);
		mRevOrder = false;
	}
	else if ( cmd.HasFlag( FLAG_REVCOLS ) ) {
		ALib::CommaList cl( cmd.GetValue( FLAG_REVCOLS ) );
		CommaListToIndex( cl, mOrder);
		mRevOrder = true;
	}
	else if ( cmd.HasFlag( FLAG_FNAMES ) ) {
		mOrderNames = cmd.GetValue( FLAG_FNAMES );
		if ( mOrderNames.Size() == 0 ) {
			CSVTHROW( "Need list of names specified by " << FLAG_FNAMES );
		}
	}
	else {
		CSVTHROW( "Problem in order comand " )
	}
}

//---------------------------------------------------------------------------

}	// namespace

// end
