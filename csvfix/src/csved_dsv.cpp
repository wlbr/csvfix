//---------------------------------------------------------------------------
// csved_dsv.cpp
//
// delimitter separated values (DSV) read/write
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_dsv.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {


//----------------------------------------------------------------------------
// Default field delimitter
//----------------------------------------------------------------------------

const char DEF_DELIM = '|';

//---------------------------------------------------------------------------
// Register read & write command
//---------------------------------------------------------------------------

static RegisterCommand <DSVReadCommand> rc1_(
	CMD_DSVR,
	"convert DSV (delimiter separated variables) data to CSV"
);

static RegisterCommand <DSVWriteCommand> rc2_(
	CMD_DSVW,
	"convert CSV to DSV format"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const DSVR_HELP = {
	"convert data in delimiter-separated variables format to CSV\n"
	"usage: csvfix read_dsv [flags] [files ...]\n"
	"where flags are:\n"
	"  -f fields\tspecify list of fields to convert (default is all)\n"
	"  -s sep\tspecify DSV separator character (default is pipe symbol)\n"
	"  -csv\t\ttreat field contents as CSV (double quotes are special)\n"
	"  -cm\t\tcollapse multiple separators into single instance\n"
	"#SMQ,IBL,OFL"
};

const char * const DSVW_HELP = {
	"convert data in CSV format to delimiter-separated variables format\n"
	"usage: csvfix write_dsv [flags] [files ...]\n"
	"where flags are:\n"
	"  -f fields\tspecify list of fields to convert (default is all)\n"
	"  -s sep\tspecify DSV separator character (default is pipe symbol)\n"
	"#IFN,IBL,OFL,SEP,SKIP"
};

//---------------------------------------------------------------------------
// Base handles field list and delimitter
//---------------------------------------------------------------------------

DSVBase ::	DSVBase( const string & name, const string & desc,
						const string &  help )
	: Command( name, desc, help ), mDelim( DEF_DELIM ) {
	AddFlag( ALib::CommandLineFlag( FLAG_SEP, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
}


//----------------------------------------------------------------------------
// Get field delimiter = default is pipe character.
//----------------------------------------------------------------------------

char DSVBase ::	Delim() const {
	return mDelim;
}

//----------------------------------------------------------------------------
// Read command line flags. The most problematic is the one that specifies
// the field separator. This defaults to '|', but any single character can be
// used. To specify a tab-separated file, use -s '\t'.
//----------------------------------------------------------------------------

void DSVBase ::	ReadFlags( const ALib::CommandLine & cl ) {
	string delim = cl.GetValue( FLAG_SEP, ALib::Str( DEF_DELIM ) );
	if ( delim.size() == 0 ) {
		CSVTHROW( "Separator specified by " << FLAG_SEP
					<< " cannot be empty" );
	}
	else if ( delim.size() == 1 ) {
		mDelim = delim[0];
	}
	else if ( delim.size() == 2 && delim[0] == '\\' ) {
		if ( delim[1] == 't' ) {
			mDelim = '\t';
		}
		else {
			CSVTHROW( "Bad escaped separator specified by " << FLAG_SEP );
		}
	}
	else {
		CSVTHROW( "Bad separator specified by " << FLAG_SEP );
	}

	string fields = cl.GetValue( FLAG_COLS, "" );
	CommaListToIndex( ALib::CommaList( fields ), mFields );
}

//----------------------------------------------------------------------------
// Convert input to output using fields list.
//----------------------------------------------------------------------------

void DSVBase ::	BuildCSVRow( const CSVRow & in, CSVRow & out ) const {

	if ( mFields.size() == 0 ) {
		out = in;
	}
	else {
		out.clear();
		for ( unsigned int i = 0; i < mFields.size(); i++ ) {
			unsigned int f = mFields[ i ];
			if ( f < in.size() ) {
				out.push_back( in[ f ] );
			}
			else {
				out.push_back( "" );
			}
		}
	}
}

//------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

DSVReadCommand :: DSVReadCommand( const string & name,
									const string & desc )
		: DSVBase( name, desc, DSVR_HELP ), mIsCSV( false ), mCollapseSep( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_CSV, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_CMULTI, false, 0 ) );

}

//---------------------------------------------------------------------------
// Read lines from input, parse them as DSV and output as CSV
//---------------------------------------------------------------------------

int DSVReadCommand :: Execute( ALib::CommandLine & cmd ) {

	ReadFlags( cmd );
	mIsCSV = cmd.HasFlag( FLAG_CSV );
	mCollapseSep = cmd.HasFlag( FLAG_CMULTI );
	IOManager io( cmd );
	CSVRow row;
	string line;

	while( io.ReadLine( line ) ) {
		ParseDSV( line, row );
		io.WriteRow( row );
	}

	return 0;
}


//----------------------------------------------------------------------------
// Helper to unquote possibly quoted CSV data.
//----------------------------------------------------------------------------

string DSVReadCommand :: Unquote( const string & s ) const {

	if ( ! mIsCSV ) {
		return s;
	}

	string t = ALib::UnQuote( s );

	if ( t.find( "\"" ) == std::string::npos  ) {
		return t;
	}

	string t2;
	for ( unsigned int i = 0; i < t.size(); i++ ) {
		char c = t.at(i);
		if ( c == '"' && ALib::Peek( t, i + 1 ) == '"' ) {
			t2 += '"';
			i++;
		}
		else {
			t2 += c;
		}
	}

	return t2;
}

//---------------------------------------------------------------------------
// Chop line up into fields seoarated by mDelim. The delimiter can be
// escaped with a backslash. If the mCollapseSep flag is set, multiple
// occurences of a separator are treated as a single one.
//---------------------------------------------------------------------------

void DSVReadCommand :: ParseDSV( const string & line, CSVRow & rv ) {

	CSVRow row;

	unsigned int pos = 0, len = line.size();
	string val;

	while( pos < len ) {
		char c = line[ pos++ ];
		if ( c == Delim() ) {
			while( mCollapseSep && ALib::Peek( line, pos ) == Delim() ) {
				pos++;
			}
			row.push_back( Unquote( val ) );
			val = "";
		}
		else if ( c == '\\' ) {
			if ( pos == len ) {
				CSVTHROW( "Escape at end of line" );
			}
			else {
				val += line[ pos++ ];
			}
		}
		else {
			val += c;
		}
	}

	row.push_back( Unquote( val )  );

	BuildCSVRow( row, rv );

}

//---------------------------------------------------------------------------
// Standard command ctor
//---------------------------------------------------------------------------

DSVWriteCommand	:: DSVWriteCommand( const string & name,
									const string & desc )
		: DSVBase( name, desc, DSVW_HELP ) {

}

//---------------------------------------------------------------------------
// Read CSV rows and output them as DSV
//---------------------------------------------------------------------------

int DSVWriteCommand :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	ReadFlags( cmd );
	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( ! Skip( row ) ) {
			io.Out() << MakeDSV( row ) << "\n";
		}
	}

	return 0;
}

//---------------------------------------------------------------------------
// Build DSV row
//---------------------------------------------------------------------------

string DSVWriteCommand :: MakeDSV( const CSVRow & in )  {

	CSVRow row;
	BuildCSVRow( in, row );

	string line;
	for ( unsigned int i = 0; i < row.size(); i++ ) {
		line += MakeField( row[i] );
		if ( i != row.size() - 1 ) {
			line += Delim();
		}
	}
	return line;
}

//---------------------------------------------------------------------------
// Create field contaents, quoting any delimiter
//---------------------------------------------------------------------------

string DSVWriteCommand :: MakeField( const string & val ) {
	if ( val.find( Delim() ) == std::string::npos  ) {
		return val;
	}
	else {
		string t;
		for ( unsigned int i = 0; i < val.size(); i++ ) {
			char c = val[i];
			if ( c == Delim() || c == '\\' ) {
				t += '\\';
			}
			t += c;
		}
		return t;
	}
}

//------------------------------------------------------------------------

} // end namespace

// end

