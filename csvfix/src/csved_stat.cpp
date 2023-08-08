//---------------------------------------------------------------------------
// csved_stat.cpp
//
// produce record/field stats for CSV files
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_stat.h"
#include "csved_strings.h"
#include <string>
#include <memory>
using std::string;

namespace CSVED {


//---------------------------------------------------------------------------
// Register stat command
//---------------------------------------------------------------------------

static RegisterCommand <StatCommand> rc1_(
	CMD_STAT,
	"produce record/field stats for CSV files"
);


//----------------------------------------------------------------------------
// Help
//----------------------------------------------------------------------------

const char * const STAT_HELP = {
	"produce record/field statistics for CSV files\n"
	"usage: csvfix stat [flags] [file ...]\n"
	"where flags are:\n"
    "  -fs\t\tproduce full record stats (default is simplified output)\n"
	"  -fn\t\tspecify that input contains a header record listing field names\n"
	"#ALL"
};

//----------------------------------------------------------------------------
// The stat command
//----------------------------------------------------------------------------

StatCommand :: StatCommand( const string & name, const string & desc )
				: Command( name, desc, STAT_HELP ) {
    AddFlag( ALib::CommandLineFlag( FLAG_FSTATS, false, 0 ) );
    AddFlag( ALib::CommandLineFlag( FLAG_FNAMES, false, 0 ) );
}


//---------------------------------------------------------------------------
// Helper to record max field lengths
//---------------------------------------------------------------------------

void RecordLengths( const CSVRow & row, std::vector <int> & lengths ) {
    for ( unsigned int i = 0; i < row.size(); i++ ) {
        if ( i >= lengths.size() ) {
            lengths.push_back( row[i].size() );
        }
        else {
            lengths[i] = std::max( lengths[i], (int) row[i].size() );
        }
    }
}

//----------------------------------------------------------------------------
// Produce full/simple file stats.
// ---------------------------------------------------------------------------

int StatCommand :: Execute( ALib::CommandLine & cmd ) {

    if ( cmd.HasFlag( FLAG_FSTATS ) ) {
        FullStats( cmd );
    }
    else {
        SimpleStats( cmd );
    }
	return 0;
}


//---------------------------------------------------------------------------
// Full stats with field names, types etc.
//---------------------------------------------------------------------------

void StatCommand :: FullStats(  ALib::CommandLine & cmd ){

    bool usehead = cmd.HasFlag( FLAG_FNAMES );
    string lastfile = "";
    IOManager io( cmd );
    CSVRow row;
    std::shared_ptr <FileStats> stats;

    while( io.ReadCSV( row ) ) {
        if ( io.CurrentFileName() != lastfile ) {
            if ( lastfile != "" ) {
                stats->Report( io );
            }
            lastfile = io.CurrentFileName();
            stats = std::make_shared<FileStats>( lastfile, usehead ? row : CSVRow() );
            if ( usehead ) {
                continue;
            }
        }
        stats->UpdateStats( row );
    }
    stats->Report( io );
}

//---------------------------------------------------------------------------
// Simple stats as per old version of stat command.
// Stat the files in the IO manager, reporting file name, number of records
// (not not number of lines!) and min and max field counts.
// Now also output max lengths of all fields.
//---------------------------------------------------------------------------

void StatCommand :: SimpleStats(  ALib::CommandLine & cmd ) {

	IOManager io( cmd );
	string filename;
	int lines = 0, fmin = INT_MAX, fmax = 0;
	CSVRow row;
    std::vector <int> lengths;

	while( io.ReadCSV( row ) ) {
		if ( filename != io.CurrentFileName() ) {
			if ( filename != "" ) {
				OutputStats( io, filename, lines, fmin, fmax, lengths );
			}
			filename = io.CurrentFileName();
			lines = 0; fmin = INT_MAX; fmax = 0; lengths.clear();
		}
		RecordLengths( row, lengths );
		lines++;
		fmin = std::min( fmin, (int) row.size() );
		fmax = std::max( fmax, (int) row.size() );
	}

	if ( filename != "" ) {
		OutputStats( io, filename, lines, fmin, fmax, lengths );
	}
}

//----------------------------------------------------------------------------
// Output stats as CSV row
//----------------------------------------------------------------------------

void StatCommand :: OutputStats( IOManager & io, const string & fname,
									int lines, int minf, int maxf,
									const std::vector <int> & lengths )  {
	CSVRow row;
	row.push_back( fname );
	row.push_back( ALib::Str( lines ) );
	row.push_back( ALib::Str( minf ) );
	row.push_back( ALib::Str( maxf ) );
	for ( int i : lengths ) {
        row.push_back( ALib::Str( i ));
    }
	io.WriteRow( row );
}

//---------------------------------------------------------------------------
// FileStats records details about an individual file - name, field names and
// individual field stats.
//---------------------------------------------------------------------------


FileStats :: FileStats( const string & filename, const CSVRow & fieldnames )
    : mFileName( filename ), mFieldNames( fieldnames ) {
}

//---------------------------------------------------------------------------
// Update detailed statistics with contents of a row.
//---------------------------------------------------------------------------

void FileStats :: UpdateStats( const CSVRow & row ) {
    for ( unsigned int i = 0; i < row.size(); i++ ) {
        if ( i >= mFieldNames.size() ) {
            mFieldNames.push_back( ALib::Str( i + 1 ) );
        }
        if ( i >= mFields.size() ) {
            mFields.push_back( FieldRecord( mFieldNames[i] ));
        }
        int len = row[i].size();
        mFields[i].mType = Transition( mFields[i].mType, row[i] );
        mFields[i].mMinLen = std::min( len, mFields[i].mMinLen );
        mFields[i].mMaxLen = std::max( len, mFields[i].mMaxLen );
    }
}

//---------------------------------------------------------------------------
// Report accumulated detailed statistics.
//---------------------------------------------------------------------------

void FileStats:: Report( IOManager & io ) const {
    for ( unsigned int i = 0; i < mFields.size(); i++ ) {
        CSVRow row;
        row.push_back( mFileName );
        row.push_back( mFieldNames[i] );
        row.push_back( FT2Str( mFields[i].mType ) );
        row.push_back( ALib::Str( mFields[i].mMinLen ) );
        row.push_back( ALib::Str( mFields[i].mMaxLen ) );
        io.WriteRow( row );
    }
}


//----------------------------------------------------------------------------

} // namespace





