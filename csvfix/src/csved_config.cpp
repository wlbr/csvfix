//---------------------------------------------------------------------------
// csved_config.cpp
//
// Configuration (aliases and defaults) for CSVfix
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_str.h"
#include "a_env.h"
#include "csved_config.h"
#include "csved_except.h"
#include "csved_strings.h"
#include "csved_version.h"
#include "csved_cli.h"
#include <fstream>

using namespace std;

//----------------------------------------------------------------------------
// Config file name and location via env variable is OS dependent, as is the 
// include file containing declaration of getcwd() & the file path separator.
//----------------------------------------------------------------------------

#ifdef _WIN32
#include <direct.h>
const string CONFIG_FILE = "csvfix.cfg";
const string HOME_VAR = "USERPROFILE";
const string PATH_SEP = "\\";
#else
#include <unistd.h>
const string CONFIG_FILE = ".csvfix";
const string HOME_VAR = "HOME";
const string PATH_SEP = "/";
#endif

//----------------------------------------------------------------------------
// Namespace can't inc direct.h without causing errors, so do not move this!
//----------------------------------------------------------------------------

namespace CSVED {

//----------------------------------------------------------------------------
// Names of config file commands and the comment prefix character
//----------------------------------------------------------------------------

const string DEF_STR = "defaults";
const string ALIAS_STR = "alias";
const char COMMENT_CHAR = '#';

//----------------------------------------------------------------------------
// Flags which are allowed as defaults - not currently used.
//----------------------------------------------------------------------------

struct DefCmdArg {
	string cmd;
	int argc;
};

DefCmdArg defargs[] = {
	{ FLAG_IGNBL, 0 },
	{ FLAG_SMARTQ, 0 },
	{ FLAG_ICNAMES, 0 },
	{ FLAG_CSVSEP, 1 },
	{ FLAG_CSVSEPR, 1 },
	{ FLAG_OUTSEP, 1 },
	{ FLAG_QLIST, 1 },
	{ "", -1 }
};

//----------------------------------------------------------------------------
// Get configuration from specified directory, which cannot be empty string.
//----------------------------------------------------------------------------

static string GetConfigNameFromDir( const string & dir ) {
	char last  = dir[dir.size() - 1];
	string sep = (last != '\\' && last != '/') ? PATH_SEP : "";
	return dir + sep + CONFIG_FILE;
}


//----------------------------------------------------------------------------
// Get the full path to the user config file. If the env variable pointing
// to the home directory is not set, don't try to use the user config file.
//----------------------------------------------------------------------------

static string GetUserConfig() {
	string dir = ALib::GetEnv( HOME_VAR );
	if ( dir.empty() ) {
		return "";
	}
	else {
		return GetConfigNameFromDir( dir );
	}
}

//----------------------------------------------------------------------------
// Get the full path to the local (current directory) config file.
//----------------------------------------------------------------------------

static string GetLocalConfig() {
	const int BUFFSIZE = 1024;     // big enough?
	char dir[ BUFFSIZE ];
	getcwd( dir, BUFFSIZE );
	return GetConfigNameFromDir( dir );
}

//----------------------------------------------------------------------------
// See if a line should be ignored - i.e. it is empty or comment
//----------------------------------------------------------------------------

static bool Ignore( const string & line ) {
	for( unsigned int i = 0; i < line.size(); i++ ) {
		if ( line[i] == COMMENT_CHAR ) {
			return true;
		}
		else if ( ! std::isspace( line[i] ) ) {
			return false;
		}
	}
	return true;
}

//----------------------------------------------------------------------------
// Populate the configuration. Use the current working directory config first,
// and if there isn't one, the user home directory config.
//----------------------------------------------------------------------------

Config :: Config( CLIHandler * cli ) : mCli( cli ) {
	if ( ! Populate( GetLocalConfig() ) ) {
		Populate( GetUserConfig() );
	}
}

//----------------------------------------------------------------------------
// Get name of current config file, or empty string if none.
//----------------------------------------------------------------------------

string Config :: ConfigFile() const {
	return mConfigFile;
}

//----------------------------------------------------------------------------
// Populate from config file
//----------------------------------------------------------------------------

bool  Config :: Populate( const string & cfg ) {
	std::ifstream f( cfg.c_str() );
	if ( ! f.is_open() ) {
		return false;
	}
	mConfigFile = cfg;
	string line;
	while( getline( f, line ) ) {
		if ( Ignore( line ) ) {
			continue;
		}
		try {
			ProcessSetting( line );
		}
		catch( const Exception & e ) {
			CSVTHROW( "In " << cfg << ": " <<  e.what() );
		}
	}
	return true;
}

//----------------------------------------------------------------------------
// Process entry in config file which will be either default options or
// an alias.
//----------------------------------------------------------------------------

void Config :: ProcessSetting( const string line ) {
	std::istringstream is( line );
	string cmd;
	is >> cmd;
	if ( cmd == DEF_STR ) {
		ProcessDefaults( is );
	}
	else if ( cmd == ALIAS_STR ) {
		ProcessAlias( is );
	}
	else {
		CSVTHROW( "Invalid configuration entry: " << line );
	}
}

//----------------------------------------------------------------------------
// Process an alias in the form:
//
//    alias alias-name cmd-name option1 option2 ...
//----------------------------------------------------------------------------

void Config :: ProcessAlias( std::istringstream & is ) {
	string alias, cmd;
	if ( ! (is >> alias) ) {
		CSVTHROW( "No alias name" );
	}
	if ( mAliases.find( alias ) != mAliases.end() ) {
		CSVTHROW( "Duplicate alias " << alias );
	}
	if ( ! (is >> cmd ) ) {
		CSVTHROW( "No command for alias " << alias );
	}
	if ( ! mCli->HasCommand( cmd ) ) {
		CSVTHROW( "No such command as " << cmd << " for alias " << alias );
	}

	string opt;
	getline( is, opt );

	mAliases.insert( std::make_pair( alias, cmd + " " + opt ) );
}

//----------------------------------------------------------------------------
// Process default options in the form:
//
//    defaults option1 option2 ...
//
// Should really validate options, but currently do not.
//----------------------------------------------------------------------------

void Config :: ProcessDefaults( std::istringstream & is ) {
	if ( mDefaults != "" ) {
		CSVTHROW( "Can specify defaults once only" );
	}
	getline( is, mDefaults );
}

//----------------------------------------------------------------------------
// See if there is an alias with the given name and return command string
// if there is one, otherwise return empty string.
//----------------------------------------------------------------------------

string Config :: GetAliasedCommand( const string & alias ) const {
	AMapType::const_iterator it = mAliases.find( alias );
	if ( it == mAliases.end() ) {
		return "";
	}
	else {
		return it->second;
	}
}

//----------------------------------------------------------------------------
// Return default options - may be empty.
//----------------------------------------------------------------------------

string Config :: Defaults() const {
	return mDefaults;
}


//---------------------------------------------------------------------------

} // end namespace


// end

