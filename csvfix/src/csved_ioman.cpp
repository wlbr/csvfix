//---------------------------------------------------------------------------
// csved_ioman.h
//
// I/O managemant for CSVfix
//
// Copyright (C) 2010 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_csv.h"
#include "a_collect.h"
#include "a_expr.h"

#include <assert.h>
#include <fstream>
#include "csved_except.h"
#include "csved_ioman.h"
#include "csved_strings.h"

using std::string;

namespace CSVED {

//----------------------------------------------------------------------------
// IOWatcher is used to monitor various changes in the IOManager
//----------------------------------------------------------------------------

IOWatcher :: ~IOWatcher() {
	// nothing
}

//---------------------------------------------------------------------------
// Create manager opening all streams to check we can, rather than fail
// after lengthy processing. Handle command line flags that don't belong
// to a specific command.
//---------------------------------------------------------------------------

IOManager :: IOManager( const ALib::CommandLine & cmdline,
							bool colmap, bool preopen  )
		: mCmdLine( cmdline ),  mInputIndex( 0 ),
			mCurrentLine( 0 ), mOutput( 0 ),
			mParser( 0),
			mSmartQuotes( false ), mSkipColNames( false ),
			mMakeColMap( colmap ),
			mCSVSep( ',' ), mRetainSep( false ), mOutputSep(0),
			mPreOpen( preopen ) {

	GetGenOpts( cmdline );
	OpenStreams();

	mIgnoreBlankLines = mCmdLine.HasFlag( FLAG_IGNBL );
	mSmartQuotes = mCmdLine.HasFlag( FLAG_SMARTQ );
	mSkipColNames = mCmdLine.HasFlag( FLAG_ICNAMES );

	if ( cmdline.HasFlag( FLAG_QLIST ) && cmdline.HasFlag( FLAG_SMARTQ )) {
		CSVTHROW( "Cannot specify both " << FLAG_QLIST
					<< " and " << FLAG_SMARTQ << " flags");

	}

	// a rather horrid hack for -sqf flag (which specifies fields that must be quoted)
	// if the QLIST option is zero, set to a really high value to prevent
	// quoting of all fields, which may result in invalid CSV output.
	if( cmdline.HasFlag( FLAG_QLIST )) {
		string ql = cmdline.GetValue( FLAG_QLIST );
		if ( ql == "0" || ql == "none" ) {
			ALib::CommaList cl( "999999"  );   // a big number
			CommaListToIndex( cl, mQuoteFields );
		}
		else {
			ALib::CommaList cl(cmdline.GetValue( FLAG_QLIST ) );
			CommaListToIndex( cl, mQuoteFields );
		}
	}

	// random number seeding for expressions
	if ( cmdline.HasFlag( FLAG_RFSEED ) ) {
		string s= cmdline.GetValue( FLAG_RFSEED );
		if ( ! ALib::IsInteger( s ) ) {
			CSVTHROW( "Seed specified by " << FLAG_RFSEED << " must be integer" );
		}
		int n = ALib::ToInteger( s );
		ALib::Expression::SetRNGSeed( n );
	}

}

//----------------------------------------------------------------------------
// Handle generic options
//----------------------------------------------------------------------------

void IOManager :: GetGenOpts( const ALib::CommandLine & cmd ) {

	NotBoth( cmd, FLAG_CSVSEP, FLAG_CSVSEPR );

	if ( cmd.HasFlag( FLAG_CSVSEP ) || cmd.HasFlag( FLAG_CSVSEPR ) ) {
		string s = cmd.GetValue(
			cmd.HasFlag( FLAG_CSVSEP ) ? FLAG_CSVSEP : FLAG_CSVSEPR );
		if ( s == "" ) {
			CSVTHROW( "CSV separator cannot be an empty string");
		}
		else if ( s.size() != 1 ) {
			CSVTHROW( "CSV separator must be a single character");
		}
		else if ( ! ALib::CSVLineParser::IsValidSep( s[0] )) {
			CSVTHROW( "Invalid CSV field separator: " << s[0] );
		}

		mCSVSep = s[0];
		mRetainSep = cmd.HasFlag( FLAG_CSVSEPR );
	}

	mHeader = cmd.GetValue( FLAG_HDRREC, "" );

	if ( cmd.HasFlag( FLAG_OUTSEP )) {
		string s = cmd.GetValue( FLAG_OUTSEP );
		if ( s == "\\t") {
			mOutputSep = '\t';
		}
		else if ( s.size() == 1 ) {
			mOutputSep = s[0];
		}
		else {
			CSVTHROW( "Invalid output separator (must be single character)");
		}
	}
}

//---------------------------------------------------------------------------
// Clear all streams we may have opened
//---------------------------------------------------------------------------

IOManager :: ~IOManager() {
	ClearStreams();
	delete mParser;
}

//----------------------------------------------------------------------------
// Add watcher which will be informed of iomanager events.
//----------------------------------------------------------------------------

void IOManager :: AddWatcher( IOWatcher & w ) {
	mWatchers.push_back( & w );
}

//---------------------------------------------------------------------------
// Delete all stored input streams (except standard input and output)
//---------------------------------------------------------------------------

void IOManager :: ClearStreams() {
	for ( unsigned int i = 0; i < mInputs.size(); i++ ) {
		if ( mInputs[i].mStream != & std::cin ) {
			delete mInputs[i].mStream;
		}
	}
	mInputs.clear();
	if ( mOutput != & std::cout ) {
		delete mOutput;
	}
	mOutput = 0;
}

//---------------------------------------------------------------------------
// How many input streams are open?
//---------------------------------------------------------------------------

unsigned int IOManager :: InStreamCount() const {
	return mInputs.size();
}

//---------------------------------------------------------------------------
// Attempt to open all streams. Input streams consist of standard input,
// or those filenames specified on command line.
// Return number of open input streams.
//---------------------------------------------------------------------------

unsigned int  IOManager :: OpenStreams() {
	ClearStreams();
	mInputIndex = 0;

	if ( mPreOpen ) {
		if (mCmdLine.FileCount() == 0 ) {
			mInputs.push_back( Input( DISP_STDIN, & std::cin ) );
		}
		else {
			for ( unsigned int i = 0; i < mCmdLine.FileCount(); i++ ) {
				OpenInputFile( mCmdLine.File( i ) );
			}
		}

		if ( InStreamCount() == 0 ) {
			CSVTHROW( "Could not open an input stream" );
		}
	}

	OpenOutputFile( mCmdLine.GetValue( FLAG_OUT, "" ) );

	return mInputs.size();
}

//----------------------------------------------------------------------------
// Create a stream parser using the input stream specified by the index. The
// caller is respomsible for deleting the stream, which is created using all
// the relevant IOManager attributes from the command line.
//----------------------------------------------------------------------------

ALib::CSVStreamParser * IOManager :: CreateStreamParser( unsigned int in ) {

	if ( in >= mInputs.size() ) {
		CSVTHROW( "Invalid sream index: " << in );
	}

	ALib::CSVStreamParser * p = new ALib::CSVStreamParser(
									In( in ),
									mIgnoreBlankLines,
									mSkipColNames,
									false,
									mCSVSep
								);
	return p;
}

//---------------------------------------------------------------------------
// Open an output stream if -o flag specified else use stdout
//---------------------------------------------------------------------------

void IOManager :: OpenOutputFile( const string & fname ) {
	if ( fname == "" ) {
		mOutput = & std::cout;
	}
	else {
		std::ofstream * ofs = new std::ofstream( fname.c_str() );
		if ( ! ofs->is_open() ) {
			delete ofs;
			CSVTHROW( "Could not open " << fname << " for output" );
		}
		mOutput = ofs;
	}
	if ( ! mHeader.empty() ) {
		(*mOutput) << mHeader << "\n";
	}
}


//---------------------------------------------------------------------------
// Write row of CSV data to ouput, doing quoting. If smart quoting is on
// then only enclose in quotes if field contains special characters. If the
// user specified the -sqf flag, always quote those fields.
//
// Added quick hack to turn CSV escaping off for use by escape command.
//---------------------------------------------------------------------------

void IOManager :: WriteRow( const CSVRow & row, bool noescape  ) {

	string SPECIAL;					// chars that need quoting

	if ( mRetainSep ) {	
		SPECIAL = "\"";
		SPECIAL += mCSVSep;
	}
	else if ( mOutputSep ) {
		SPECIAL += mOutputSep;
		SPECIAL += '"';
	}
	else {
		SPECIAL = "\",";
	}


	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( mSmartQuotes
				&& row[i].find_first_of( SPECIAL ) == string::npos ) {
			Out() << row[i];
		}
		else if ( mQuoteFields.size() ) {
			if ( ALib::Contains( mQuoteFields, i )) {
				Out() << ALib::CSVQuote( row[i] );
			}
			else {
				Out() << row[i];
			}
		}
		else if ( noescape ) {
			Out() << '"' << row[i] << '"';
		}
		else {
			Out() << ALib::CSVQuote( row[i] );
		}
		if ( i != row.size() - 1 ) {
			if ( mOutputSep ) {
				Out() << mOutputSep;
			}
			else {
				Out() << ( mRetainSep ? mCSVSep : ',' );
			}
		}
	}
	Out() << "\n";
}

//---------------------------------------------------------------------------
// Get current input filename & line number
//---------------------------------------------------------------------------

string IOManager :: CurrentFileName() const {
	return InFileName( mInputIndex );
}

unsigned int IOManager :: CurrentLine() const {
	return mCurrentLine;
}

string IOManager :: CurrentInput() const {
	return mCurrentInput;
}

//---------------------------------------------------------------------------
// Read single line from the input streams returning true until they are all
// exhausted at which point return false.
//---------------------------------------------------------------------------

bool IOManager :: ReadLine( string & line ) {

	while( mInputIndex < mInputs.size() ) {
		if ( std::getline( In( mInputIndex ), line ) ) {
			mCurrentLine++;
			mCurrentInput = line;
			if ( mIgnoreBlankLines && ALib::IsEmpty( line ) ) {
				continue;
			}
			return true;
		}
		else {
			mCurrentLine = 0;
			mCurrentInput = "";
			mInputIndex++;
		}
	}
	return false;
}

//---------------------------------------------------------------------------
// Parse CSV input into a row container. Returns true while there is more
// input to come. Need to cause a new csv file event on first read of
// every new file.
//---------------------------------------------------------------------------

bool IOManager :: ReadCSV( CSVRow & row ) {

	while( mInputIndex < mInputs.size() ) {
		static bool needevent = false;
		if ( mParser == 0 ) {
			mParser = new ALib::CSVStreamParser( In( mInputIndex ),
													mIgnoreBlankLines,
													mSkipColNames,
													mMakeColMap,
													mCSVSep );
			needevent = true;
		}

		if ( mParser->ParseNext( row ) ) {
			if ( needevent ) {
				// inform watchers that new CSV stream has started
				for ( unsigned int i = 0; i < mWatchers.size(); i++ ) {
					mWatchers[i]->OnNewCSVStream(
									mInputs[mInputIndex].mFileName,
									mParser );
				}
				needevent = false;
			}
			mCurrentLine = mParser->LineNo();
			mCurrentInput = mParser->RawLine();
			return true;
		}
		else {
			delete mParser;
			mParser = 0;
			mCurrentLine = 0;
			mCurrentInput = "";
			mInputIndex++;
		}
	}
	return false;
}

//---------------------------------------------------------------------------
// Open a named file for input. Use '-' to specify stdinput.
//---------------------------------------------------------------------------

void IOManager :: OpenInputFile( const string & fname ) {
	if ( fname == NAME_STDIN ) {
		for ( unsigned int i = 0; i < mInputs.size(); i++ ) {
			if ( mInputs[i].mFileName == DISP_STDIN ) {
				CSVTHROW( "Can use " << NAME_STDIN << " once only" );
			}
		}
		mInputs.push_back( Input( DISP_STDIN, & std::cin ) );
	}
	else {
		std::ifstream * ifs = new std::ifstream( fname.c_str() );
		if ( ! ifs->is_open() ) {
			delete ifs;
			CSVTHROW( "Cannot open " << fname << " for input" );
		}
		mInputs.push_back( Input( fname, ifs ));
	}
}

//---------------------------------------------------------------------------
// Get input stream specified by index.
//---------------------------------------------------------------------------

std::istream & IOManager :: In( unsigned int index ) const {
	return * mInputs.at( index ).mStream;
}

//---------------------------------------------------------------------------
// Get file name of indexed stream
//---------------------------------------------------------------------------

string IOManager :: InFileName( unsigned int index ) const {
	return mInputs.at( index ).mFileName;
}

//---------------------------------------------------------------------------
// Get single current output stream
//---------------------------------------------------------------------------

std::ostream & IOManager :: Out() const {
	assert( mOutput != 0 );
	return * mOutput;
}

//---------------------------------------------------------------------------

} // end namespace

// end


