//---------------------------------------------------------------------------
// a_html.cpp
//
// html page renering for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------


#include <time.h>
#include <assert.h>
#include "a_except.h"
#include "a_html.h"
#include <map>

using std::string;
using std::map;

namespace ALib {

//----------------------------------------------------------------------------
// Convert commonly used special characters to entities.
//----------------------------------------------------------------------------

string HTMLToEntity( const std::string & html ) {
	static map <char, string> m;
	if ( m.size() == 0 ) {
		m.insert( std::make_pair( '&', "&amp;"));
		m.insert( std::make_pair( '<', "&lt;"));
		m.insert( std::make_pair( '>', "&gt;"));
		m.insert( std::make_pair( '"', "quot;"));
	}

	if ( html.find_first_of( "&<>\"" ) == std::string::npos  ) {
		return html;
	}
	else {
		string t;
		for ( unsigned int i = 0; i < html.size(); i++ ) {
			map <char,string>::const_iterator it = m.find( html[i] );
			if ( it == m.end() ) {
				t += html[i];
			}
			else {
				t += it->second;
			}
		}
		return t;
	}
}


} // namespace


#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_html" );

DEFTEST( EnvTest ) {
	string s1 = "One & One is < Three";
	string s2 = HTMLToEntity( s1 );
	FAILNE( s2, "One &amp; One is &lt; Three" );
	string s3 = "One & One";
	s2 = HTMLToEntity( s3 );
	FAILNE( s2, "One &amp; One" );
}

#endif


// end

