//---------------------------------------------------------------------------
// csved_filesplit.cpp
//
// split CSV stream into multiple files depending on field
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "csved_filesplit.h"
#include "csved_cli.h"
#include "csved_strings.h"
#include "a_collect.h"

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register filesplit command
//---------------------------------------------------------------------------

static RegisterCommand <FileSplitCommand> rc1_(
	CMD_FSPLIT,
	"split into files"
);

//----------------------------------------------------------------------------
// Default filename and path components. These can be changed by the user
// via command line parameters.
//----------------------------------------------------------------------------

const char * const DEF_DIR 	= "";			// directory path
const char * const DEF_PREF = "file_";		// file name prefix
const char * const DEF_EXT 	= "csv";		// file extension

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const SPLIT_HELP = {
	"splits data into multiple files depending on CSV field contents\n"
	"usage: csvfix file_split  [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields \tfields to use as basis for split\n"
	"  -fd dir\tdirectory to place output files in  (default is current)\n"
	"  -fp pre\tprefix to use to generate filenames(default is file_)\n"
	"  -fx ext\textension to use to generate filenames(default is csv)\n"
	"  -ufn\t\tuse field content to generate filenames\n"
	"#IBL,SEP,IFN,SMQ,SKIP,PASS"
};

//---------------------------------------------------------------------------
// Usual ctor stuff
//---------------------------------------------------------------------------

FileSplitCommand :: FileSplitCommand( const string & name,
							const string & desc )
			: Command( name, desc, SPLIT_HELP), mUseFieldNames( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FSPRE, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FSDIR, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FSEXT, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_USEFLD, false, 0 ) );

	mFileNo = 1;
}

//----------------------------------------------------------------------------
// Read input and split into separate files
//----------------------------------------------------------------------------

int FileSplitCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}
		if ( Pass( row ) ) {
			io.WriteRow( row  );
		}
		else {
			WriteRow( io, row );
		}
	}

	mOutFile.flush();

	return 0;
}

//----------------------------------------------------------------------------
// Create key consisting of the various field values separated by null chars
//----------------------------------------------------------------------------

string FileSplitCommand :: MakeKey( const CSVRow & row ) {
	string key;
	for ( unsigned int i = 0; i < row.size() ; i++ ) {
		if ( ALib::Contains( mColIndex, i ) ) {
			key += row[i];
			key += '\0';
		}
	}
	return key;
}

//----------------------------------------------------------------------------
// See if we already have a row with same key as this one. If we do, write
// the row to the same file as the existing row, otherwise creater a new
// file name. If the file name is the same as the the one used on the last
// call, re-use the file else open a new one.
//----------------------------------------------------------------------------

void FileSplitCommand :: WriteRow( IOManager & ioman, const CSVRow & row ) {

	string key = MakeKey( row );
	const string * p = mDict.GetPtr( key );
	string fname = (p == 0) ? NewFileName( key ) : * p;

	if ( p == 0 ) {
		mDict.Add( key, fname );
	}

	if ( fname != mCurrentFile ) {
		mOutFile.close();
		mOutFile.clear();	// close does not reset state
		mOutFile.open( fname.c_str(), std::ios::out | std::ios::app );   // append

		if ( ! mOutFile.is_open() ) {
			CSVTHROW( "Could not open file " << fname << " for output" );
		}
		mCurrentFile = fname;
	}

	mOutFile << ioman.CurrentInput() << "\n";
}

//----------------------------------------------------------------------------
// Generate new unique file name. If the -ufn flag was specified, use the
// key value to generate the file names, otherwise number them sequentially.
// Assumes the i/o library is happy with forward slash path separators.
//----------------------------------------------------------------------------

const int FILE_NUM_LEN = 4;    // how many digits for numbered files

string FileSplitCommand :: NewFileName( const string & key ) {
	string f;
	if ( mDir != "" ) {
		f += mDir + "/";
	}
	f += mFilePrefix;

	if ( mUseFieldNames ) {
		for ( unsigned int i = 0; i < key.size() - 1; i++ ) {
			f += key[i] == 0 ? '_' : key[i];
		}
	}
	else {
		f += ALib::ZeroPad( mFileNo++, FILE_NUM_LEN );
	}
	f += ".";
	f += mFileExt;
	return f;
}

//----------------------------------------------------------------------------
// Handle the various flags. Only a list of fields to split on is req'd.
//----------------------------------------------------------------------------

void FileSplitCommand :: ProcessFlags( ALib::CommandLine & cmd ) {
	mDir = cmd.GetValue( FLAG_FSDIR, DEF_DIR );
	mFilePrefix = cmd.GetValue( FLAG_FSPRE, DEF_PREF );
	mFileExt= cmd.GetValue( FLAG_FSEXT, DEF_EXT );
	ALib::CommaList cl( cmd.GetValue( FLAG_COLS ) );
	CommaListToIndex( cl, mColIndex );
	mUseFieldNames = cmd.HasFlag( FLAG_USEFLD );
}

//----------------------------------------------------------------------------

} // namespace


// end

