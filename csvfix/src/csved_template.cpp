//---------------------------------------------------------------------------
// csved_template.cpp
//
// templated output for csved
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "csved_cli.h"
#include "csved_template.h"
#include "csved_strings.h"
#include "csved_except.h"
#include "a_expr.h"
#include <fstream>

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register template command
//---------------------------------------------------------------------------

static RegisterCommand <TemplateCommand> rc1_(
	CMD_TPLATE,
	"output via template"
);

//----------------------------------------------------------------------------
// Parameter intro/outro and expression indicator
//----------------------------------------------------------------------------

const char PINTRO = '{';
const char POUTRO = '}';
const char EVALCHR = '@';

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const TPL_HELP = {
	"formatted output using template file (output is not CSV)\n"
	"usage: csvfix template [flags] [files ...]\n"
	"where flags are:\n"
	"  -ft file\tspecify name of template file\n"
	"  -fn ftpl\tspecify template for generating file names\n"
	"#IBL,SEP,IFN,OFL,SKIP"
};

//---------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

TemplateCommand ::	TemplateCommand( const string & name,
									 const string & desc )
		: Command( name, desc, TPL_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_TFILE, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FNAMES, false, 1 ) );
}

//---------------------------------------------------------------------------
// Put all input through template to format output
//---------------------------------------------------------------------------

int TemplateCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ReadTemplate( cmd );

	if ( cmd.HasFlag( FLAG_FNAMES ) ) {
		mFileTemplate = cmd.GetValue( FLAG_FNAMES );
	}

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}

		if ( mFileTemplate.empty() ) {
			io.Out() << ReplaceColumns( mTemplate, row );
		}
		else {
			FileOut( row );
		}
	}

	return 0;
}

//----------------------------------------------------------------------------
// Output to a file specified by the -fn option, which itself is a template.
//----------------------------------------------------------------------------

void TemplateCommand :: FileOut( const CSVRow & row  ) {
	string fname = ReplaceColumns( mFileTemplate, row );
	std::ofstream ifs( fname );
	if ( ! ifs.is_open() ) {
		CSVTHROW( "Cannot open file " << fname << " for output" );
	}
	ifs << ReplaceColumns( mTemplate, row );
}

//---------------------------------------------------------------------------
// Handles the special characters backslash and open brace. Backslash
// does C-style quoting of the next character. Open brace introduces
// a formatting placeholder. For exanple {2} will be replaced by column
// two from the input row. Append placeholder value to the 'out'
// parameter. Also updates the 'pos' param with new position in template.
//
// If the string in braces begins with the special EVALCHR character, it is
// an expression in the expression language and needs to be evaluated.
//---------------------------------------------------------------------------

void TemplateCommand :: HandleSpecialChars( const string & tplate,
											char c, unsigned int & pos,
											const CSVRow & row,
											string & out ) {
	char t = tplate[ pos++ ];
	if ( c == '\\' ) {
		switch( t ) {
			case '\n':
			case '\r':	CSVTHROW( "Invalid escape at end of line" );
			case 'n':	out += '\n'; break;
			case 't':	out += '\t'; break;
			default:	out += t;
		}
	}
	else {		// its a brace
		string ns;
		while( t != POUTRO) {
			if ( t == '\n' || t == '\r' ) {
				CSVTHROW( "Missing closing brace" );
			}
			ns += t;
			t = tplate[ pos++ ];
		}

		if( ns.size() && ns[0] == EVALCHR ) {    // it's an expression
			out += Eval( row, ns );
			return;
		}

		if ( ! ALib::IsInteger( ns ) ) {
			CSVTHROW( "Invalid placeholder: " << "{" << ns << "}" );
		}

		int n =  ALib::ToInteger( ns ) - 1;		// to zero based
		if ( n < 0 ) {
			CSVTHROW( "Invalid placeholder: " << "{" << ns << "}" );
		}

		if ( (unsigned int) n < row.size() ) {
			out += row[ n ];
		}
	}
}

//----------------------------------------------------------------------------
// Evaluate a string as an expression. The string will start with EVALCHR
// which needs to be removed.
//----------------------------------------------------------------------------

string TemplateCommand :: Eval( const CSVRow & row, const string & expr ) {
	ALib::Expression ex;
	for( unsigned int i = 0; i < row.size(); i++ ) {
		ex.AddPosParam( row[i] );
	}
	return ex.Evaluate( expr.substr( 1 ) );
}

//---------------------------------------------------------------------------
// Replace all placeholders enclosed in braces in the template with the
// coresponding column from therow. If there is no such column, ignore.
// Other characters are copied literaly to output.
//---------------------------------------------------------------------------

string TemplateCommand :: ReplaceColumns( const string & tplate,
											const CSVRow & row ) {

	if ( ALib::IsEmpty( tplate ) ) {
		CSVTHROW( "No template contents" );
	}

	unsigned int pos = 0, len = tplate.size();

	string out;

	while( pos != len ) {
		char c = tplate[pos++];
		if ( c == '\\' || c == PINTRO ) {
			HandleSpecialChars( tplate, c, pos, row, out );
		}
		else {
			out += c;
		}
	}

	return out;
}

//---------------------------------------------------------------------------
// Read template file into mTemplate member
//---------------------------------------------------------------------------

void TemplateCommand :: ReadTemplate( const ALib::CommandLine & cmd ) {
	string fname = cmd.GetValue( FLAG_TFILE );
	if ( ALib::IsEmpty( fname ) ) {
		CSVTHROW( "Need template file name specified with "
						<< FLAG_TFILE << " flag" );
	}
	std::ifstream ifs( fname.c_str() );
	if ( ! ifs.is_open() ) {
		CSVTHROW( "Cannot open file " << fname << "  for input" );
	}

	mTemplate = "";
	string line;
	while( std::getline( ifs, line ) ) {
		mTemplate += line + "\n";
	}

	ifs.close();
}

//------------------------------------------------------------------------

} // end namespace

// end

