//---------------------------------------------------------------------------
// csved_valid.cpp
//
// validation for CSVED
//
// Copyright (C) 2009 Neil Butterwqorth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "a_sort.h"
#include "csved_cli.h"
#include "csved_valid.h"
#include "csved_strings.h"
#include <fstream>
#include <ctype.h>

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Register validate  command
//---------------------------------------------------------------------------

static RegisterCommand <ValidateCommand> rc1_(
	CMD_VALID,
	"validate CSV data files"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const VALID_HELP = {
	"validate CSV input against rules (output is not CSV)\n"
	"usage: csvfix validate [flags] [file ...]\n"
	"where flags are:\n"
	"  -vf file\tspecify file containing validation rules\n"
	"  -om mode\toutput mode (pass,fail,report)\n"
	"#IFN,SEP,OFL,IBL,SKIP"
};

//------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

ValidateCommand ::	ValidateCommand( const string & name,
										const string & desc )
		: Command( name, desc, VALID_HELP ), mOutMode( Reports ) {
	AddFlag( ALib::CommandLineFlag( FLAG_VFILE, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_OMODE, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ERRCODE, false, 0 ) );
}

//---------------------------------------------------------------------------
// Needed because we own the created rules
//---------------------------------------------------------------------------

ValidateCommand ::	~ValidateCommand() {
	Clear();
}

//---------------------------------------------------------------------------
// Free the rules we created
//---------------------------------------------------------------------------

void ValidateCommand :: Clear() {
	ALib::FreePtrs( mRules );
	mRules.clear();
}

//---------------------------------------------------------------------------
// Read inputs and for each row apply rules.
//---------------------------------------------------------------------------

int ValidateCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	GetOutputMode( cmd );
	ReadValidationFile( cmd );

	IOManager io( cmd );
	CSVRow row;

	// we optionally return an error code to the shell if validation failed
	int errtotal = 0;
	bool errcode = cmd.HasFlag( FLAG_ERRCODE );

	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}
		int errcount = 0;
		for ( unsigned int i = 0; i < mRules.size(); i++ ) {
			ValidationRule::Results res = mRules[i]->Apply( row );

			if ( res.size() && mOutMode == Reports ) {
				Report( io, res, errcount );
				errcount++;
				errtotal += errcount;
				continue;
			}

			if ( res.size() ) {
				errcount++;
				if ( mOutMode == Fails ) {
					io.WriteRow( row );
					break;
				}
			}
		}
		if ( mOutMode == Passes && errcount == 0 ) {
			io.WriteRow( row );
		}
		errtotal += errcount;
	}

    // exit code of 2 indicates program detected invalid data
	return errtotal && errcode ? 2 : 0;
}

//---------------------------------------------------------------------------
// Get mode for output. This can be one of:
//
//		report	- give file/lineno/err msg reports
//		pass	- list CSV rows that pass validation
//		fail	- list CSV rows that fail validation
//---------------------------------------------------------------------------

void ValidateCommand :: GetOutputMode( const ALib::CommandLine & cl ) {
	string om = cl.GetValue( FLAG_OMODE, "report" );
	if ( om == "report" ) {
		mOutMode = Reports;
	}
	else if ( om == "pass" ) {
		mOutMode = Passes;
	}
	else if ( om == "fail" ) {
		mOutMode = Fails;
	}
	else {
		CSVTHROW( "Invalid value for " << FLAG_OMODE << ": " << om );
	}
}

//---------------------------------------------------------------------------
// Report any validation errors
//---------------------------------------------------------------------------

void ValidateCommand :: Report( IOManager & io,
								const ValidationRule::Results & res,
								int errcount ) const {
	if ( res.size() == 0 ) {
		return;
	}
	else {
		if ( errcount == 0 ) {
			io.Out() << io.CurrentFileName() << " (" << io.CurrentLine () << "): ";
			io.Out() << io.CurrentInput() << "\n";
		}
		for ( unsigned int i = 0; i < res.size(); i++ ) {
			if ( res[i].Field() > 0 ) {
				io.Out() << "    field: " << res[i].Field() << " - ";
			}
			else {
				io.Out() << "    ";
			}
			io.Out() << res[i].Msg() << "\n";
		}
	}
}

//---------------------------------------------------------------------------
// Read validation specification file, creating validation rules.
//---------------------------------------------------------------------------

void ValidateCommand :: ReadValidationFile( const ALib::CommandLine & cmd ) {

	Clear();

	string fname = cmd.GetValue( FLAG_VFILE );
	if ( fname == "" ) {
		CSVTHROW( "Need validation file specified by "
					<< FLAG_VFILE << " flag" );
	}

	std::ifstream ifs( fname.c_str() );
	if ( ! ifs.is_open() ) {
		CSVTHROW( "Cannot open validation file " << fname << " for input" );
	}

	string line;
	while( std::getline( ifs, line ) ) {
		if ( ALib::IsEmpty( line ) || line[0] == '#' ) {	// a comment
			continue;
		}

		unsigned int pos = 0;
		string name = ReadName( line, pos );
		FieldList flist = ReadFields( line, pos );
		ValidationRule::Params params = ReadParams( line, pos );

		ValidationRule * rule = RuleFactory::CreateRule( name, flist, params );
		if ( rule == 0 ) {
			CSVTHROW( "Unknown rule: " << name );
		}

		//rule->DumpOn( std::cout );

		mRules.push_back( rule );

	}
}

//---------------------------------------------------------------------------
// Read rule name from validation faile
//---------------------------------------------------------------------------

string ValidateCommand :: ReadName( const string & line,
										unsigned int & pos ) const {
	string name;
	while( pos < line.size() && ! isspace( line[pos] ) ) {
		name += line[pos++];
	}
	pos++;		// skip possible space
	return name;
}

//---------------------------------------------------------------------------
// Read comma-separated list of fields from validation file.
//---------------------------------------------------------------------------

FieldList ValidateCommand :: ReadFields( const string & line,
											unsigned int & pos ) const {
	SkipSpaces( line, pos );

	FieldList fl;
	if ( pos < line.size() && line[pos] == '*' ) {
		pos++;
		return fl;
	}

	string sf;
	while( pos < line.size() && ! isspace( line[pos] ) ) {
		sf += line[pos++];
	}
	pos++;

	vector <string> tmp;

	ALib::Split( sf, ',', tmp );

	for ( unsigned int i = 0; i < tmp.size(); i++ ) {
		if ( ALib::IsInteger( tmp[i] ) ) {
			int n = ALib::ToInteger( tmp[i] );
			if ( n > 0 ) {
				fl.push_back ( n - 1 );
				continue;
			}
		}
		CSVTHROW( "Invalid field list: " << sf );
	}

	if ( fl.size() == 0 ) {
		CSVTHROW( "Need at least one field in rule: " << line );
	}

	return fl;
}

//---------------------------------------------------------------------------
// Read validationn rule parameters - there may not be any
//---------------------------------------------------------------------------

ValidationRule::Params  ValidateCommand :: ReadParams(
											const string & line,
											unsigned int & pos ) const {
	ValidationRule::Params params;
	while( 	SkipSpaces( line, pos ) ) {
		char c = line[pos];
		if ( c == '\'' || c == '"' ) {
			ReadQuotedString( params, line, pos );
		}
		else {
			ReadWSTermString( params, line, pos );
		}
	}

	return params;
}

//---------------------------------------------------------------------------
// Read string enclosed in single or double quotes, discarding quotes.
// Add string to the params vector.
//---------------------------------------------------------------------------

void ValidateCommand :: ReadQuotedString( ValidationRule::Params & params,
											const string & line,
											unsigned int  & pos ) const{
	string s;
	unsigned int start = pos;
	char quote = line[pos++];

	while( pos < line.size() ) {
		char c = line[pos++];
		if ( c == quote ) {
			params.push_back( s );
			return;
		}
		s += c;
	}
	CSVTHROW( "Unterminated quoted value: " << line.substr( start ) );
}

//---------------------------------------------------------------------------
// Read string terminated by whitespace and add to params.
//---------------------------------------------------------------------------

void ValidateCommand :: ReadWSTermString( ValidationRule::Params & params,
								const std::string & line,
								unsigned int  & pos ) const {
	string s;
	while( pos < line.size() && ! isspace( line[pos] )) {
		s += line[pos++];
	}
	params.push_back( s );
}


//---------------------------------------------------------------------------
// Skip spaces before a validation file entry. Return true if there is
// something more to read.
//---------------------------------------------------------------------------

bool ValidateCommand :: SkipSpaces( const string & line,
										unsigned int & pos ) const {
	while( pos < line.size() ) {
		if ( ! isspace( line[pos] ) ) {
			return true;
		}
		pos++;
	}
	return false;
}

//------------------------------------------------------------------------

} // end namespace

// end

