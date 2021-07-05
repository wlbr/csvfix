//---------------------------------------------------------------------------
// csved_atable.cpp
//
// convert csv input to ascii art tables
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_atable.h"
#include "csved_strings.h"
#include <iostream>

using std::string;
using std::vector;

namespace CSVED {

//----------------------------------------------------------------------------
// Register command
//----------------------------------------------------------------------------

static RegisterCommand <AsciiTableCommand> rc1_(
	CMD_ATABLE,
	"produce ascii table"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const ATABLE_HELP = {
	"format input CSV data as 'ASCII art' table (output is not CSV)\n"
	"usage: csvfix echo [flags] [file ...]\n"
	"where flags are:\n"
	"  -h head\tcomma-separated list of table headers\n"
	"\t\tuse '-h @' to interpret first input line as the header\n"
	"  -ra fields\tlist of field indexes to right-align\n"
	"  -s\t\tinsert separator after every line of data\n"
	"#IBL,IFN,OFL,SKIP"

};

//----------------------------------------------------------------------------
// If the file header is the following special symbol, then read the header
// from the first line of the file.
//----------------------------------------------------------------------------

const string FILE_HEADER = "@";

//----------------------------------------------------------------------------
// standard command constructor
//----------------------------------------------------------------------------

AsciiTableCommand :: AsciiTableCommand( const string & name,
								const string & desc )
		: Command( name, desc, ATABLE_HELP ), mUseLineSep( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_HEADER, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_RALIGN, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_SEP, false, 0 ) );
}

//----------------------------------------------------------------------------
// read all rows into memory to allow us to calc max widths
// then write them all out in table format
//----------------------------------------------------------------------------

int AsciiTableCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );
	IOManager io( cmd );
	CSVRow row;
	while( io.ReadCSV( row ) ) {
		if ( ! Skip( row ) ) {
			AddRow( row );
		}
	}
	OutputTable( io.Out() );
	return 0;
}

//----------------------------------------------------------------------------
// create a separator row to begin & terminate table
//----------------------------------------------------------------------------

string AsciiTableCommand :: MakeSep() const {
	string sep = "+";
	for ( unsigned int i = 0; i < mWidths.size(); i++ ) {
		sep += "-" + ALib::RightPad( "", mWidths[i], '-' ) + "-+";
	}
	return sep;
}

//----------------------------------------------------------------------------
// headings will be the first row
//----------------------------------------------------------------------------

void AsciiTableCommand :: OutputHeadings( std::ostream & os,
											const CSVRow & row ) {
	os << "|";
	for ( unsigned int i = 0; i < mWidths.size() ; i++ ) {
		if ( i < row.size() ) {
			os << " "  << ALib::Centre( row[i], mWidths[i] ) << " |";
		}
		else {
			os << " " << ALib::RightPad( "", mWidths[i] ) << " |";
		}
	}
	os << "\n";
	os << MakeSep() << "\n";
}

//----------------------------------------------------------------------------
// table consists of a separator, optional header + separator, data,
// and a closing separator
//----------------------------------------------------------------------------

void AsciiTableCommand :: OutputTable( std::ostream & os ) {

	string sep = MakeSep();

	os << sep <<" \n";
	for ( unsigned int i = 0; i < mRows.size() ; i++ ) {
		if ( i == 0 && mHeadings.Size() != 0 ) {
			OutputHeadings( os, mRows[i] );
		}
		else {
			OutputRow( os, mRows[i] );
			if ( mUseLineSep ) {
				os << sep << "\n";
			}
		}
	}
	if ( ! mUseLineSep ) {
		os << sep <<"\n";
	}
}

//----------------------------------------------------------------------------
// output row data padded appropriately
//----------------------------------------------------------------------------

void AsciiTableCommand :: OutputRow( std::ostream & os, const CSVRow & row ) {
	os << "|";
	for ( unsigned int i = 0; i < mWidths.size() ; i++ ) {
		if ( i < row.size() ) {
			if ( ALib::Contains( mRightAlign, i ) ) {
				os << " "  << ALib::LeftPad( row[i], mWidths[i] ) << " |";
			}
			else {
				os << " "  << ALib::RightPad( row[i], mWidths[i] ) << " |";
			}
		}
		else {
			os << " " << ALib::RightPad( "", mWidths[i] ) << " |";
		}
	}
	os << "\n";
}

//----------------------------------------------------------------------------
// add row to stored rows, recording widths
//----------------------------------------------------------------------------

void AsciiTableCommand :: AddRow( const CSVRow & row ) {
	unsigned int n = row.size();
	while( mWidths.size() < n ) {
		mWidths.push_back( 0 );
	}
	for ( unsigned int i = 0; i < n ; i++ ) {
		if ( mWidths[i] < row[i].size() ) {
			mWidths[i] = row[i].size();
		}
	}
	mRows.push_back( row );
}

//----------------------------------------------------------------------------
// set user flags -m if header specified, make it first row
//----------------------------------------------------------------------------

void AsciiTableCommand :: ProcessFlags( ALib::CommandLine & cmd ) {
	mUseLineSep = cmd.HasFlag( FLAG_SEP );
	mHeadings = ALib::CommaList( cmd.GetValue( FLAG_HEADER, "" ) );
	ALib::CommaList ra = ALib::CommaList( cmd.GetValue( FLAG_RALIGN, "" ) );
	CommaListToIndex( ra, mRightAlign );

	if ( mHeadings.Size() && mHeadings.At(0) != FILE_HEADER) {
		CSVRow r;
		for ( unsigned int i = 0; i < mHeadings.Size() ; i++ ) {
			r.push_back( mHeadings.At( i ) );
		}
		AddRow( r );
	}
}

//----------------------------------------------------------------------------

} // namespace

// end

