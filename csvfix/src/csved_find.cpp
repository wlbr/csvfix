//---------------------------------------------------------------------------
// csved_find.cpp
//
// regex searching for csvfix
// regexes are implemented in alib and are very basic
// now also supports searches for non-regex ranges
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_find.h"
#include "csved_strings.h"
#include "csved_evalvars.h"

#include "a_debug.h"

using std::string;
using std::vector;

namespace CSVED {


//---------------------------------------------------------------------------
// Register find & remove commands
//---------------------------------------------------------------------------

static RegisterCommand <FindCommand> rc1_(
	CMD_FIND,
	"find rows matching regular expression or range"
);

static RegisterCommand <FindCommand> rc2_(
	CMD_REMOVE,
	"remove rows matching regular expression or range"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const FIND_HELP = {
	"perform grep-like search using regular expressions on CSV data\n"
	"usage: csvfix find  [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to search\n"
	"  -e expr\tregex to search for - multiple  -e flags are allowed\n"
	"  -s expr\tas for -e, but don't treat expr as regex\n"
	"  -fc count\tspecify field count to find\n"
	"  -r range\trange to search for - multiple -r flags are allowed\n"
	"  -ei expr\tas for -e flag, but search ignoring case\n"
	"  -si expr\tas for -e flag, but don't treat expr as regex\n"
	"  -n\t\toutput count of matched rows only\n"
	"  -l length\tsearch for fields of given length (may be a range)\n"
	"  -if expr\tonly output line if eval expression evaluates to true\n"
};

const char * const REMOVE_HELP = {
	"perform grep-like search on CSV data, but return non-matching records\n"
	"usage: csvfix remove [flags] [file ...]\n"
	"where flags are:\n"
	"  -f fields\tfields to search\n"
	"  -e expr\tregex to search for - multiple  -e flags are allowed\n"
	"  -s expr\tas for -e, but don't treat expr as regex\n"
	"  -fc count\tspecify field count to remove\n"
	"  -r range\trange to search for - multiple -r flags are allowed\n"
	"  -ei expr\tas for -e flag, but search ignoring case\n"
	"  -si expr\tas for -e flag, but don't treat expr as regex\n"
	"  -n\t\toutput count of non-matching rows only\n"
	"  -l length\t search for fields of given length (may be a range)\n"
	"  -if expr\tdon't output line if eval expression evaluates to true\n"
};

//------------------------------------------------------------------------
// Standard comand ctor
//---------------------------------------------------------------------------

FindCommand :: FindCommand( const string & name,
							const string & desc )

		: Command( name, desc ), mRemove( name == CMD_REMOVE ),
			mCountOnly( false ),
			mMinFields( 0 ), mMaxFields( INT_MAX ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_EXPR, false, 1, true ) );
	AddFlag( ALib::CommandLineFlag( FLAG_STR, false, 1, true ) );
	AddFlag( ALib::CommandLineFlag( FLAG_RANGE, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_EXPRIC, false, 1, true ) );
	AddFlag( ALib::CommandLineFlag( FLAG_STRIC, false, 1, true ) );
	AddFlag( ALib::CommandLineFlag( FLAG_NUM, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_LEN, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FCOUNT, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_IF, false, 1 ) );
}

//---------------------------------------------------------------------------
// Release regexes etc.
//---------------------------------------------------------------------------

FindCommand :: 	~FindCommand() {
	Clear();
}

//----------------------------------------------------------------------------
// Help for both commands
// ??? Needs to be fixed to use #SMQ et al. ???
//----------------------------------------------------------------------------

string FindCommand :: Help() const {
	static const string ghelp =
		"  -ibl\t\tignore blank input lines\n"
		"  -ifn\t\tignore field name record\n"
		"  -smq\t\tuse smart quotes on output\n"
		"  -sep sep\tspecify CSV field separator character\n"
		"  -o file\twrite output to file, rather than standard output\n";
	if ( mRemove ) {
		return REMOVE_HELP + ghelp;
	}
	else {
		return FIND_HELP + ghelp;
	}
}

//---------------------------------------------------------------------------
// Read all rows from input and match them against regexes. If the -n flag
// is used, output ionly the row count.
//---------------------------------------------------------------------------

int FindCommand :: Execute( ALib::CommandLine & cmd ) {

	Clear();
	ALib::CommaList cl( cmd.GetValue( FLAG_COLS ) );
	CommaListToIndex( cl, mColIndex );
	CreateRegExes( cmd );
	CreateRanges( cmd );
	CreateLengths( cmd );
	CreateFieldCounts( cmd );

	if ( cmd.HasFlag( FLAG_IF ) ) {
		string e = cmd.GetValue( FLAG_IF, ""  );
		string emsg = mEvalExpr.Compile( e );
		if ( emsg != "" ) {
			CSVTHROW( emsg + " " + e );
		}

	}


	if ( (! HaveRegex())
			&& (! cmd.HasFlag( FLAG_FCOUNT ))
			&& (! cmd.HasFlag( FLAG_IF )) ) {
		CSVTHROW( "Need at least one " << FLAG_EXPR
					<< ", " << FLAG_RANGE
					<< ", " << FLAG_LEN
					<< ", " << FLAG_FCOUNT
					<< ", " << FLAG_IF
					<< " or "<< FLAG_EXPRIC << " flag" );
	}

	mCountOnly = cmd.HasFlag( FLAG_NUM );

	IOManager io( cmd );
	CSVRow row;

	unsigned int count = 0;
	while( io.ReadCSV( row ) ) {
		if ( mEvalExpr.IsCompiled() ) {
			AddVars( mEvalExpr, io, row );
			bool es = ALib::Expression::ToBool( mEvalExpr.Evaluate() );
			if ( (es && mRemove) || (!es && !mRemove)) {
				continue;
			}
		}
		// check field count is in range and remove if necessary
		if ( cmd.HasFlag( FLAG_FCOUNT ) ) {
			bool fcok = int(row.size()) >= mMinFields
							&& int(row.size()) <= mMaxFields;
			if ( (mRemove && fcok) || ( ! mRemove && ! fcok ) ) {
				continue;
			}

		}

		bool match = HaveRegex() ? MatchRow( row ) : ! mRemove;
		if ( mRemove ^ match ) {		// only one must be true
			count++;
			if ( ! mCountOnly ) {
				io.WriteRow( row );
			}
		}
	}

	if ( mCountOnly ) {			// count is not CSV
		io.Out() << count << "\n";
	}

	return 0;
}


//----------------------------------------------------------------------------
// Has user specified some sort of regexor range?
//----------------------------------------------------------------------------

bool FindCommand :: HaveRegex() const {
	return mExprs.size() != 0 || mRanges.size() != 0  || mLengths.size() != 0;
}

//---------------------------------------------------------------------------
// Try to match 's' against all compiled regexes.
//---------------------------------------------------------------------------

bool FindCommand :: TryAllRegExes( const string & s ) {
	for ( unsigned int i = 0; i < mExprs.size(); i++ ) {
		ALib::RegEx::Pos pos = mExprs[i]->FindIn( s );
		if ( pos.Found() ) {
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------
// Try to match 's' against available ranges.
//----------------------------------------------------------------------------

bool FindCommand :: TryAllRanges( const string & s ) {
	for ( unsigned int i = 0; i < mRanges.size(); i++ ) {
		if ( mRanges[i].mIsNum ) {
			if ( ALib::IsNumber( s ) ) {
				double ds = ALib::ToReal( s );
				double d1 = ALib::ToReal( mRanges[i].mRange.first );
				double d2 = ALib::ToReal( mRanges[i].mRange.second );
				if ( ds >= d1 && ds <= d2 ) {
					return true;
				}
			}
		}
		else {
			if ( s >= mRanges[i].mRange.first
					&& s <= mRanges[i].mRange.second ) {
				return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------------
// See if length of s passes length tests
//----------------------------------------------------------------------------

bool FindCommand :: TryAllLengths( const string & s ) {
	int len = s.size();
	for ( unsigned int i = 0; i < mLengths.size(); i++ ) {
		if ( len >= mLengths[i].first && len <= mLengths[i].second ) {
			return true;
		}
	}
	return false;
}

//---------------------------------------------------------------------------
// Match a row using column index info
//---------------------------------------------------------------------------

bool FindCommand :: MatchRow( CSVRow & row ) {
	for ( unsigned int i = 0; i < row.size(); i++ ) {
		if ( mColIndex.size() == 0 || ALib::Contains( mColIndex, i ) ) {
			if ( TryAllRegExes( row[i] )
					|| TryAllRanges( row[i] )
					|| TryAllLengths( row[i] ) ) {
				return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------------
// Templated helper to check valid range
//----------------------------------------------------------------------------

template <typename T>
void CheckRange( const T & a, const T & b ) {
	if ( a > b ) {
		CSVTHROW( "Invalid range: " << a << ":" << b );
	}
}

//----------------------------------------------------------------------------
// Parse the -fc flag to get field counts to find
//----------------------------------------------------------------------------

void FindCommand :: CreateFieldCounts( const ALib::CommandLine & cmd ) {
	mMinFields = 0;
	mMaxFields = INT_MAX;
	if ( !cmd.HasFlag( FLAG_FCOUNT )) {
		return;
	}
	vector <string> fc;
	ALib::Split( cmd.GetValue( FLAG_FCOUNT ), ':', fc );
	if ( fc.size() == 1 ) {
		fc.push_back( fc[0] );
	}
	else if ( fc.size() == 2 ) {
		if ( fc[0] == "" ) {
			if ( fc[1] == "" ) {
				CSVTHROW( "Invalid range for " << FLAG_FCOUNT << " flag" );
			}
			fc[0] = "0";
		}
		else if ( fc[1] == "" ) {
			fc[1] = "1000000";
		}
	}
	else {
		CSVTHROW( "Invalid field count for " << FLAG_FCOUNT << " flag" );
	}
	if ( ! (ALib::IsInteger( fc[0] ) && ALib::IsInteger( fc[1] ) ) ) {
		CSVTHROW( "Field counts must be integers for " << FLAG_FCOUNT );
	}
	mMinFields = ALib::ToInteger( fc[0] );
	mMaxFields = ALib::ToInteger( fc[1] );
	if ( mMinFields > mMaxFields ) {
		CSVTHROW( "invalid field count specified by " << FLAG_FCOUNT );
	}
}

//----------------------------------------------------------------------------
// Create lengths - may be specified as a single number or as range
//----------------------------------------------------------------------------

void FindCommand :: CreateLengths( const ALib::CommandLine & cmd ) {
	for ( int i = 2; i < cmd.Argc(); i++ ) {	// skip exe name & command
		string flag = cmd.Argv( i );
		if ( flag != FLAG_LEN ) {
			continue;
		}
		if ( i == cmd.Argc() - 1 ) {
			CSVTHROW( "No length following " << flag << " flag" );
		}

		string r = cmd.Argv( ++i );	// get length  and skip it
		vector <string> rs;
		ALib::Split( r, ':', rs );
		if ( rs.size() == 1 ) {
			rs.push_back( rs[0] );     // make single length a range
		}

		if ( rs.size() != 2
				|| ! ALib::IsInteger( rs[0] )
				|| ! ALib::IsInteger( rs[1] ) ) {
			CSVTHROW( "Invalid range: " << r );
		}

		int n1 = ALib::ToInteger( rs[0] );
		int n2 = ALib::ToInteger( rs[1] );

		if ( n1 < 0 || n2 < 0 || n1 > n2 ) {
			CSVTHROW( "Invalid range: " << r );
		}
		mLengths.push_back( std::make_pair( n1, n2 ));
	}
}

//----------------------------------------------------------------------------
// Build all ranges from command line.
//----------------------------------------------------------------------------

void FindCommand :: CreateRanges( const ALib::CommandLine & cmd ) {
	for ( int i = 2; i < cmd.Argc(); i++ ) {	// skip exe name & command
		string flag = cmd.Argv( i );
		if ( flag != FLAG_RANGE ) {
			continue;
		}
		if ( i == cmd.Argc() - 1 ) {
			CSVTHROW( "No range following " << flag << " flag" );
		}

		string r = cmd.Argv( ++i );	// get range and skip it
		vector <string> rs;
		if ( ALib::Split( r, ':', rs ) != 2 ) {
			CSVTHROW( "Invalid range: " << r );
		}

		bool isnum = false;
		if ( ALib::IsNumber( rs[0] ) && ALib::IsNumber(rs[1])) {
			double d1 = ALib::ToReal( rs[0] );
			double d2 = ALib::ToReal( rs[1] );
			CheckRange( d1, d2 );
			isnum = true;
		}
		else {
			CheckRange(  rs[0], rs[1]);
		}

		mRanges.push_back( Range( std::make_pair( rs[0], rs[1] ), isnum ));
	}
}

//---------------------------------------------------------------------------
// Get regexes from command line and compile them. Three flags are possible;
// -e (normal regex), -ei (ignore case regex), -si (respect case, not regex)
//---------------------------------------------------------------------------

void FindCommand :: CreateRegExes( const ALib::CommandLine & cmd ) {
	for ( int i = 2; i < cmd.Argc(); i++ ) {	// skip exe name & command
		string flag = cmd.Argv( i );
		if ( flag != FLAG_EXPR && flag != FLAG_STR
				&& flag != FLAG_EXPRIC  && flag != FLAG_STRIC ) {
			continue;
		}
		if ( i == cmd.Argc() - 1 ) {
			CSVTHROW( "No expression following " << flag << " flag" );
		}

		string es = cmd.Argv( ++i );	// get expr and skip it
		ALib::RegEx::CaseSense cs = (flag == FLAG_STR || flag == FLAG_EXPR
										|| flag == FLAG_STRIC )
									? ALib::RegEx::Sensitive
									: ALib::RegEx::Insensitive;

		ALib::RegEx * rep = new ALib::RegEx( flag == FLAG_STRIC || flag == FLAG_STR
												? ALib::RegEx::Escape( es )
												: es, cs );
		mExprs.push_back( rep );
	}
}

//---------------------------------------------------------------------------
// Free compiled regexes
//---------------------------------------------------------------------------

void FindCommand :: Clear() {
	ALib::FreePtrs( mExprs );
	mExprs.clear();
	mColIndex.clear();
}


//------------------------------------------------------------------------

} // end namespace

// end

