//------------------------------------------------------------------------
// csved_command.cpp
//
// Base class for CSVED commands
//
// Copyright (C) 2008 Neil Butterworth
//------------------------------------------------------------------------


#include "a_str.h"
#include "a_csv.h"
#include "csved_command.h"
#include "csved_except.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//----------------------------------------------------------------------------
// This is the help for the generic flags
//----------------------------------------------------------------------------

const char * const GEN_SEED = "  -seed n\tseed to be used by random() function\n";
const char * const GEN_SEP = "  -sep s\tspecify CSV field separator character\n";
const char * const GEN_RSEP = "  -rsep s\tas for -sep but retain separator on output\n";
const char * const GEN_OSEP = "  -osep s\tspecifies output separator\n";
const char * const GEN_IBL = "  -ibl\t\tignore blank input lines\n";
const char * const GEN_IFN = "  -ifn\t\tignore field name record\n";
const char * const GEN_SMQ = "  -smq\t\tuse smart quotes on output\n";
const char * const GEN_SQF = "  -sqf fields\tspecify fields that must be quoted\n";
const char * const GEN_OFL	= "  -o file\twrite output to file "
									"rather than standard output\n";

const char * const GEN_SKIP = "  -skip t\tif test t is true, do not process or output record\n";
const char * const GEN_PASS = "  -pass t\tif test t is true, output CSV record as is\n";

const char * const GEN_HDR ="  -hdr s\twrite the string s out as a header record\n";

//------------------------------------------------------------------------
// Construct from command name, short description and list of flags
// that can be used with this command.
//---------------------------------------------------------------------------

Command :: Command( const string & name,
						const string & desc)
	: mName( name ), mDesc( desc )   {
}

Command :: Command( const string & name,
						const string & desc,
						const string & help )
	: mName( name ), mDesc( desc ), mHelp( help ){
}

//---------------------------------------------------------------------------
// Do-nothing dtor
//---------------------------------------------------------------------------

Command :: ~Command() {
	// nothing
}

//---------------------------------------------------------------------------
// accessors
//---------------------------------------------------------------------------

string Command :: Name() const {
	return mName;
}

string Command :: Desc() const {
	return mDesc;
}


void Command :: AddHelp( const string & help ) {
	mHelp = help;
}

//----------------------------------------------------------------------------
// Expressions to test whether an input row should be skipped or passed
// through as is.
//----------------------------------------------------------------------------

void Command :: GetSkipOptions( const ALib::CommandLine & cl ) {

	mSkipExpr = ALib::Expression();
	mPassExpr = ALib::Expression();

	if ( cl.HasFlag( FLAG_SKIP ) ) {
		string expr = cl.GetValue( FLAG_SKIP );
		mSkipExpr.Compile( expr );
	}
	if ( cl.HasFlag( FLAG_PASS ) ) {
		string expr = cl.GetValue( FLAG_PASS );
		mPassExpr.Compile( expr );
	}
}

static void SetPosParams( ALib::Expression & e, const CSVRow & r ) {
	e.ClearPosParams();
	for( unsigned int i = 0; i < r.size(); i++ ) {
		e.AddPosParam( r[i] );
	}
}

static bool EvalSkipPass( ALib::Expression & e, const CSVRow & r ) {
	if ( e.IsCompiled() ) {
		SetPosParams( e, r );
		string ev = e.Evaluate();
		return ev == "0" ? false : true;
	}
	else {
		return false;
	}
}

bool Command :: Skip( const CSVRow & r  )  {
	return EvalSkipPass( mSkipExpr, r );
}

bool Command :: Pass( const CSVRow & r )  {
	return EvalSkipPass( mPassExpr, r );
}

//----------------------------------------------------------------------------
// Process help text. May contain a terminal section preceded by a # char
// containing names of the generic flags applicable to this command. The
// name "ALL" means the command can support all generic flags.
//
// ToDo: This is a mess and needs rethinking. But it does "work".
//----------------------------------------------------------------------------

const char GFL_DELIM = '#';

string Command :: Help() const {

	if ( mHelp == "" ) {	// no help written yet
		return "";
	}

	vector <string> tmp;
	ALib::Split( mHelp, GFL_DELIM, tmp );
	if ( tmp.size() > 1 ) {
		tmp[0] += "\n";
		if ( tmp[1].find( "ALL" ) != string::npos ) {
			tmp[1] +="IBL,IFN,SMQ,OFL,SEP";
		}
		if ( tmp[1].find( "IBL" ) != string::npos ) {
			tmp[0] += GEN_IBL;
		}
		if ( tmp[1].find( "SEP" ) != string::npos ) {
			// if you can specify a CSV separator you can specify & retain
			tmp[0] += GEN_SEP;
			tmp[0] += GEN_RSEP;
			tmp[0] += GEN_OSEP;
			tmp[0] += GEN_HDR;
		}
		if ( tmp[1].find( "IFN" ) != string::npos ) {
			tmp[0] += GEN_IFN;
		}
		if ( tmp[1].find( "SMQ" ) != string::npos ) {
			// if you can specify smart quotes, you can specify a quote list
			tmp[0] += GEN_SMQ;
			tmp[0] += GEN_SQF;
		}
		if ( tmp[1].find( "OFL" ) != string::npos ) {
			tmp[0] += GEN_OFL;
		}
		if ( tmp[1].find( "SKIP" ) != string::npos ) {
			tmp[0] += GEN_SKIP;
		}
		if ( tmp[1].find( "PASS" ) != string::npos ) {
			tmp[0] += GEN_PASS;
		}
	}
	return tmp[0];
}

//---------------------------------------------------------------------------
// Give my flags to the command line & then get it to check them
//---------------------------------------------------------------------------


void Command :: CheckFlags( ALib::CommandLine & cmd ) {

	for ( unsigned int i = 0; i < mFlags.size(); i++ ) {
		cmd.AddFlag( mFlags[i] );
	}

	// all commands can have output flag and can ignore blank lines
	// also now can have smart quotes and ignore col header
	cmd.AddFlag( ALib::CommandLineFlag( FLAG_OUT, false, 1 ) );
	cmd.AddFlag( ALib::CommandLineFlag( FLAG_IGNBL, false, 0 ) );
	cmd.AddFlag( ALib::CommandLineFlag( FLAG_SMARTQ, false, 0 ) );
	cmd.AddFlag( ALib::CommandLineFlag( FLAG_ICNAMES, false, 0 ) );
	cmd.AddFlag( ALib::CommandLineFlag( FLAG_CSVSEP, false, 1 ) );
	cmd.AddFlag( ALib::CommandLineFlag( FLAG_CSVSEPR, false, 1 ) );
	cmd.AddFlag( ALib::CommandLineFlag( FLAG_OUTSEP, false, 1 ) );
	cmd.AddFlag( ALib::CommandLineFlag( FLAG_QLIST, false, 1 ) );
	cmd.AddFlag( ALib::CommandLineFlag( FLAG_RFSEED, false, 1 ) );
	cmd.AddFlag( ALib::CommandLineFlag( FLAG_HDRREC, false, 1 ) );

	vector <string> tmp;
	ALib::Split( mHelp, GFL_DELIM, tmp );
	if ( tmp.size() > 1 && tmp[1].find( "SKIP" ) != string::npos ) {
		cmd.AddFlag( ALib::CommandLineFlag( FLAG_SKIP, false, 1 ) );
		cmd.AddFlag( ALib::CommandLineFlag( FLAG_PASS, false, 1 ) );
	}

	cmd.CheckFlags( 2 );		// don't check the command name

}

//----------------------------------------------------------------------------
// Provide count of flags excluding the generic ones
//----------------------------------------------------------------------------

int Command :: CountNonGeneric( const ALib::CommandLine & cmd ) const  {

	int n = 0;

	for ( int i = 0; i < cmd.Argc(); i++ ) {
		if ( ALib::StrFirst( cmd.Argv(i) ) == '-' ) {
			string arg = cmd.Argv( i );
			if ( arg != FLAG_OUT
				&& arg != FLAG_IGNBL
				&& arg != FLAG_SMARTQ
				&& arg != FLAG_CSVSEP
				&& arg != FLAG_OUTSEP
				&& arg != FLAG_PASS
				&& arg != FLAG_ICNAMES ) {
					n++;
			}
		}
	}

	return n;
}

//---------------------------------------------------------------------------
// Add flag to allowed flags for a command
//---------------------------------------------------------------------------

void Command :: AddFlag( const ALib::CommandLineFlag & f ) {
	mFlags.push_back( f );
}

//------------------------------------------------------------------------

}	// end namespace

// end
