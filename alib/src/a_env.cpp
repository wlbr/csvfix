//---------------------------------------------------------------------------
// a_env.cpp
//
// environment & command line stuff for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#include <stdlib.h>
#include "a_base.h"
#include "a_assert.h"
#include "a_dict.h"
#include "a_except.h"
#include "a_str.h"
#include "a_env.h"
#include <map>
#include <cstdlib>

// need win api if doing our own globbing on windows
#include "a_win.h"
#include "a_dir.h"

using std::string;
using std::vector;

//---------------------------------------------------------------------------
// Alib stuff starts here
//---------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// Single command line instance
//---------------------------------------------------------------------------

CommandLine * CommandLine::mInstance = 0;

//---------------------------------------------------------------------------
// Transform args into vector of strings and set instance pointer
//---------------------------------------------------------------------------

CommandLine :: CommandLine( int argc, char * argv[] )
	: mArgs( argv, argv + argc ) {
	if ( mInstance != 0 ) {
		ATHROW( "Only one CommandLine object allowed" );
	}
	mInstance = this;
}

//---------------------------------------------------------------------------
// Reset instance pointer
//---------------------------------------------------------------------------

CommandLine :: ~CommandLine() {
	mInstance = 0;
}

//---------------------------------------------------------------------------
// Get command line instance
//---------------------------------------------------------------------------

CommandLine * CommandLine :: Instance() {
	if ( mInstance == 0 ) {
		ATHROW( "No CommandLine object exists" );
	}
	return mInstance;
}

//---------------------------------------------------------------------------
// How many args - same as arc in main()
//---------------------------------------------------------------------------

int CommandLine :: Argc() const {
	return mArgs.size();
}

//---------------------------------------------------------------------------
// Get arg by integer index - asame as argv[] in main
//---------------------------------------------------------------------------

const string & CommandLine :: Argv( int i ) const {
	if ( i < 0 || i >= Argc() ) {
		ATHROW( "Invalid command line parameter index: " << i );
	}
	return mArgs[i];
}

//----------------------------------------------------------------------------
// Clear all args and files
//----------------------------------------------------------------------------

void CommandLine :: Clear() {
	mArgs.clear();
	mFiles.clear();
}

//----------------------------------------------------------------------------
// Add arg to args list
//----------------------------------------------------------------------------

void CommandLine :: Add( const std::string & arg ) {
	mArgs.push_back( arg );
}

//---------------------------------------------------------------------------
// Get index of flag, returning -1 if does not exist
//---------------------------------------------------------------------------

int CommandLine :: FindFlag( const string & name ) const {
	for ( int i = 0; i < Argc(); i++ ) {
		if ( Argv(i) == name ) {
			return i;
		}
	}
	return -1;
}

//---------------------------------------------------------------------------
// How many occurences of a flag are there?
//---------------------------------------------------------------------------

unsigned int CommandLine :: FlagCount( const string & name ) const {
	unsigned int count = 0;
	for ( int i = 0; i < Argc(); i++ ) {
		if ( Argv(i) == name ) {
			count++;
		}
	}
	return count;
}

//---------------------------------------------------------------------------
// See if flag exists - this (and all other flag functions) is
// case sensitive and requires the exact flag including leading
// dashes or whatever.
//---------------------------------------------------------------------------

bool CommandLine :: HasFlag( const string & name ) const {
	return FindFlag( name ) >= 0;
}

//---------------------------------------------------------------------------
// Get value for parameter of flag or default value if param does not exist
//---------------------------------------------------------------------------

string CommandLine :: GetValue( const string & name,
								const string & defval ) const {
	int i = FindFlag( name );
	if ( i < 0 || i == Argc() - 1 ) {
		return defval;
	}
	else {
		return Argv( i + 1 );
	}
}

//---------------------------------------------------------------------------
// Get all values for a particular flag
//---------------------------------------------------------------------------

unsigned int CommandLine :: GetValues( const string & name,
						vector <string> & vals,
						const string & defval ) const {

	for ( int i = 1; i < Argc(); i++ ) {
		if ( Argv(i) == name ) {
			string val = i < Argc() - 1 ? Argv( i + 1 ) : defval;
			if ( val != "" && val[0] == '-' ) {
				val = defval;
			}
			vals.push_back( val );
		}
	}
	return vals.size();
}

//---------------------------------------------------------------------------
// Get value for flag or throw if flag does not exist
//---------------------------------------------------------------------------

string CommandLine :: MustGetValue( const string & name ) const {
	int i = FindFlag( name );
	if ( i < 0 || i == Argc() - 1 ) {
		ATHROW( "Required value for option " << name << " not found" );
	}
	return Argv( i + 1 );
}


//---------------------------------------------------------------------------
// Add a flag to list of flags that the command line knows about
//---------------------------------------------------------------------------

void CommandLine :: AddFlag( const CommandLineFlag & f ) {
	if  ( mFlagDict.Contains( f.Name() ) ) {
		ATHROW( "Duplicate option: " << f.Name() );
	}
	mFlagDict.Add( f.Name(), f );
}

//---------------------------------------------------------------------------
// User-callable function to check that the command line uses only the
// specified flags. Also gets the list of filenames from the command line.
// Throws exceptions if validation of flags fails.
//---------------------------------------------------------------------------

void CommandLine :: CheckFlags( unsigned int start ) {

	CheckRequiredFlags( start );
	CheckMultiFlags( start );

	int pos = start;
	while( pos < Argc() ) {
		string a = Argv( pos );
		if ( ! IsEmpty( a ) ) {
			if ( a == "-" || a[0] != '-' ) {	// stdio dash or not flag
				CheckNoMoreFlags( pos );
				break;
			}
			else {
				const CommandLineFlag * f = mFlagDict.GetPtr( a );
				if ( f == 0 ) {
					ATHROW( "Invalid option: " << a );
				}
				else if ( f->ParamCount() ) {
					if ( pos + 1 == Argc() ) {
						ATHROW( "Option has no value: " << a );
					}
					string param = Argv( ++pos  );
				}
			}
		}
		pos++;
	}

	BuildFileList( start );
}

//---------------------------------------------------------------------------
// See if flag exists on command line starting at 'start' - this lets us
// ignore things that aren't flags, such as sub-commands.
//---------------------------------------------------------------------------

bool CommandLine :: HasFlag( const string & name, unsigned int start ) {
	for ( int i = start; i < Argc(); i++ ) {
		if ( Argv( i ) == name ) {
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------------
// Check that any multiple uses of same flag are allowed
//---------------------------------------------------------------------------

void CommandLine :: CheckMultiFlags( unsigned int start ) {

	std::map <string, int> flagcount;

	for ( int i = start; i < Argc(); i++ ) {
		string f = Argv(i);
		if ( f.size() && f != "-" && f[0] == '-' ) {
			flagcount[f]++;
		}
	}

	std::map <string,int>::const_iterator it = flagcount.begin();
	while( it != flagcount.end() ) {
		if ( it->second > 1 ) {
			const CommandLineFlag * f = mFlagDict.GetPtr( it->first );
			if ( f && ! f->MultipleOK() ) {
				ATHROW( "Multiple " << f->Name() << " options not allowed" );
			}
		}
		++it;
	}
}

//---------------------------------------------------------------------------
// Check that flags that are required are present
//---------------------------------------------------------------------------

void CommandLine :: CheckRequiredFlags( unsigned int start ) {
	vector <string> names;
	mFlagDict.GetNames( names );
	for ( unsigned int i = 0; i < names.size(); i++ ) {
		const CommandLineFlag * f = mFlagDict.GetPtr( names[i] );
		if ( f->Required() && ! HasFlag( names[i], start ) ) {
			ATHROW( "Required option " << names[i] << " missing" ;)
		}
	}
}

//---------------------------------------------------------------------------
// Check that there are no more flags on the command line after 'start'
//---------------------------------------------------------------------------

void CommandLine :: CheckNoMoreFlags( unsigned int start ) {
	int pos = start;
	while( pos < Argc() ) {
		string a = Argv( pos );
		if ( ! IsEmpty( a ) && a != "-" && a[0] == '-' ) {
			ATHROW( "Unexpected value " << SQuote( Argv( start ) ) );
		}
		pos++;
	}
}

//---------------------------------------------------------------------------
// Construct list of files on command line. File names come after
// any flags, so we work backwards until we get a flag.
// On Windows, we need to do our own filename globbing, as we need
// not to glob parameters for regex flags.
//---------------------------------------------------------------------------

unsigned int CommandLine :: BuildFileList( unsigned int start ) {

	mFiles.clear();

	// work backwards to find last flag
	unsigned int pos = Argc() - 1, lastflag = 0;
	while( pos >= start ) {
		string arg = Argv( pos );
		if ( arg.size() && arg != "-" && (arg.at(0) == '-' && ! isdigit( Peek( arg, 1 )))) {
			lastflag = pos;
			break;
		}
		pos--;
	}

	unsigned int filestart = 0;
	if ( lastflag ) {
		// if there was a flag then start is one past flag
		// plus any paramaeter count
		const CommandLineFlag * f = mFlagDict.GetPtr( Argv( lastflag ) );
		AASSERT( f != 0 );	// we already validated flags
		filestart = lastflag + 1 + f->ParamCount();
	}
	else {
		// otherwise its right at the start
		filestart = start;
	}

	// save all file names
	for ( int i = filestart; i < Argc(); i++ ) {
#ifdef ALIB_WINAPI			// do our own globbing for windows
    DirList dir( Argv( i ) );
	if ( dir.Count() == 0 ) {
		mFiles.push_back( Argv( i ) );
	}
	else {
		for( unsigned int i = 0; i < dir.Count(); i++ ) {
			mFiles.push_back( dir.At(i)->Name() );
		}
	}
#else
    // on linux/unix we have already globbed
	mFiles.push_back( Argv( i ) );
#endif
	}

	return mFiles.size();
}


//---------------------------------------------------------------------------
// How many files?
//---------------------------------------------------------------------------

unsigned int CommandLine :: FileCount() const {
	return mFiles.size();
}

//---------------------------------------------------------------------------
// Get a file.
//---------------------------------------------------------------------------

string CommandLine :: File( unsigned int i ) const {
	if ( i >= mFiles.size() ) {
		ATHROW( "Invalid CommandLine file index: " << i );
	}
	return mFiles[ i ];
}

//---------------------------------------------------------------------------
// Command line flags have a name (e.g. -f), an indicator of whether they
// are required or not, and the number of parameters they can have (this
// can currently only be 1 or 0).
//---------------------------------------------------------------------------


CommandLineFlag	:: CommandLineFlag( const string & name,
									bool reqd,
									unsigned int pcount,
									bool multi  )
		: mName( name ), mRequired( reqd ),
			mParamCount( pcount ), mMulti( multi ) {
	if ( pcount > 1 ) {
		ATHROW( "Invalid CommandLine parameter count: " << pcount );
	}
}

//---------------------------------------------------------------------------
// Accessors
//---------------------------------------------------------------------------

string CommandLineFlag	:: Name() const {
	return mName;
}

bool CommandLineFlag :: Required() const {
	return mRequired;
}

unsigned int CommandLineFlag :: ParamCount() const  {
	return mParamCount;
}

bool CommandLineFlag :: MultipleOK() const {
	return mMulti;
}

//---------------------------------------------------------------------------
// Populate the dictionary from command line environment pointer
//---------------------------------------------------------------------------

Environment :: Environment( char * env[] ) {
	unsigned int i = 0;
	while( env[i] ) {
		mDict.AddNVP( env[i] );
		i++;
	}
}

//---------------------------------------------------------------------------
// Get value by name, or empty string if doesn't exist
//---------------------------------------------------------------------------

string Environment :: Get( const string & name ) const {
	return mDict.Get( name, "" );
}

//---------------------------------------------------------------------------
// See if we have env variable
//---------------------------------------------------------------------------

bool Environment :: Contains( const string & name ) const {
	return mDict.Contains( name );
}

//---------------------------------------------------------------------------
// Dump environment for debug
//---------------------------------------------------------------------------

void Environment :: DumpOn( std::ostream & os ) const {
	mDict.DumpOn( os );
}

//----------------------------------------------------------------------------
// Simple functions to get env values without using objects.
//----------------------------------------------------------------------------

string GetEnv( const string & ev ) {
	const char * val = std::getenv( ev.c_str() );
	return val == 0 ? "" : val;
}

bool EnvContains( const string & ev ) {
	return std::getenv( ev.c_str() ) != 0;
}

//---------------------------------------------------------------------------

}	// namespace

//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_env" );

DEFTEST( EnvTest ) {
	FAILNE( EnvContains( "PATH" ),  true );
	FAILNE( GetEnv( "PATH" ).size() > 0, true  );
}

#endif

// end

