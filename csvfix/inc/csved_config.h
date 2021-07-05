//---------------------------------------------------------------------------
// csved_config.h
//
// Configuration (aliases and defaults) for csvfix
//
// Copyright (C) 2012 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_CONFIG_H
#define INC_CSVED_CONFIG_H

#include "a_base.h"
#include "a_dict.h"
#include "a_env.h"
#include "csved_command.h"

#include <string>
#include <map>
#include <sstream>

namespace CSVED {

//----------------------------------------------------------------------------

class CLIHandler;

class Config {

	public:

		Config( CLIHandler * cli );
		std::string GetAliasedCommand( const std::string & alias ) const;
		std::string Defaults() const;
		std::string ConfigFile() const;

	private:

		bool Populate( const std::string & cfg );
		void ProcessSetting( const std::string line );
		void ProcessAlias( std::istringstream & is );
		void ProcessDefaults( std::istringstream & is );

		CLIHandler * mCli;
		std::string mDefaults, mConfigFile;
		typedef std::map <std::string, std::string> AMapType;
		AMapType mAliases;

};

//---------------------------------------------------------------------------

}	// namespace

#endif
