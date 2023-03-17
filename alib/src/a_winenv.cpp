//----------------------------------------------------------------------------
// a_winenv.cpp
//
// split out from a_env.cpp as it contains windows-specificstuff
//
// Copyright (C) 2014 Neil Butterworth
//----------------------------------------------------------------------------

#include <stdlib.h>
#include "a_base.h"
#include "a_assert.h"
#include "a_dict.h"
#include "a_except.h"
#include "a_str.h"
#include "a_env.h"
#include <map>
#include <cstdlib>

#include "a_dir.h"
#include "a_win.h"

using std::string;

namespace ALib {

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
        DirList dir( Argv( i ) );
        if ( dir.Count() == 0 ) {
            mFiles.push_back( Argv( i ) );
        }
        else {
            for( unsigned int i = 0; i < dir.Count(); i++ ) {
                mFiles.push_back( dir.At(i)->Name() );
            }
        }
    }
	return mFiles.size();
}

} // namespace
