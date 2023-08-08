//---------------------------------------------------------------------------
// csved_split.cpp
//
// split single field to multiple fields
// can split on:
//
//      specific character
//      transitions between char types
//      field position and length
//      field length (with one variable length field)
//      regular expression match
//
// Copyright (C) 2014 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_split.h"
#include "csved_strings.h"

using std::string;
using std::vector;

namespace CSVED {

//----------------------------------------------------------------------------
// Transition split options
//----------------------------------------------------------------------------

const char * const FLAG_TRANA2N	= "-tan";   // from alpha to digit
const char * const FLAG_TRANN2A	= "-tna";   // from digit to alpha

//---------------------------------------------------------------------------
// Register split commands
//---------------------------------------------------------------------------

static RegisterCommand <SplitFixed> rc1_(
	CMD_SPLFIX,
	"split input using fixed sizes"
);

static RegisterCommand <SplitChar> rc2_(
	CMD_SPLCHR,
	"split using regular expression"
);

static RegisterCommand <SplitRegex> rc3_(
	CMD_SPLREGEX,
	"split at character or character type transition"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const FSPLIT_HELP = {
	"split CSV field at fixed positions\n"
	"usage: csvfix split_fixed  [flags] [file ...]\n"
	"where flags are:\n"
	"  -f field\tindex of the field to be split\n"
	"  -p plist\tlist of positions to split, in start:len format\n"
	"  -l lengths\tlengths of fields to split (mutually exclusive with -p)\n"
	"  -k\t\tretain field being split in output (default is discard it)\n"
	"#ALL,SKIP,PASS"
};

const char * const RESPLIT_HELP = {
	"split using regular expression\n"
	"usage: csvfix split_fixed  [flags] [file ...]\n"
	"where flags are:\n"
	"  -f field\tindex of the field to be split\n"
	"  -r regex\tregular expression to use to perform split\n"
	"  -ic\t\tignore case when matching regular expressions\n"
	"  -k\t\tretain field being split in output (default is discard it)\n"
	"#ALL,SKIP,PASS"
};
const char * const CSPLIT_HELP = {
	"split CSV field at specific character(s) or character type transition\n"
	"usage: csvfix split_char  [flags] [file ...]\n"
	"where flags are:\n"
	"  -f field\tindex of the field to be split (required)\n"
	"  -c char\tcharacter to split at (default is space)\n"
	"  -tcn\tsplit at first transition from character to number\n"
	"  -tnc\tsplit at first transition from number to character\n"
	"  -k\t\tretain field being split in output (default is discard it)\n"
	"#ALL,SKIP,PASS"
};

//---------------------------------------------------------------------------
// Standard Command ctor
//---------------------------------------------------------------------------

SplitBase :: SplitBase( const string & name,
						const string & desc,
						const string & help )
		: Command( name, desc, help ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_KEEP, false, 0 ) );
}

//---------------------------------------------------------------------------
// Get flags common to all split commands
//---------------------------------------------------------------------------

void SplitBase :: GetCommonFlags( ALib::CommandLine & cl ) {
	mKeep = cl.HasFlag( FLAG_KEEP );
	string nf = cl.GetValue( FLAG_COLS, "" );
	if ( ! ALib::IsInteger( nf ) ) {
		CSVTHROW( "Field specified by " << FLAG_COLS << " must be integer" );
	}
	if ( ALib::ToInteger( nf ) - 1 < 0 ) {
		CSVTHROW( "Invalid field index: " << nf );
	}
	mField = ALib::ToInteger( nf ) - 1;
}

//---------------------------------------------------------------------------
// Helper to insert the split fields into the row at the position
// specified by the -f flag.
//---------------------------------------------------------------------------

void SplitBase :: Insert( CSVRow & row, const CSVRow & split ) {
	CSVRow tmp;
	for ( unsigned int j = 0; j < row.size(); j++ ) {
		if ( j == Field() ) {
			for ( unsigned int k = 0; k < split.size(); k++ ) {
				tmp.push_back( split[k] );
			}
			if ( Keep() ) {
				tmp.push_back( row[j] );
			}
		}
		else {
			tmp.push_back( row[j] );
		}
	}
	row.swap( tmp );
}

//---------------------------------------------------------------------------
// Split via regular expression
//---------------------------------------------------------------------------

SplitRegex :: SplitRegex( const string & name,
							const string & desc )
	: SplitBase( name, desc, RESPLIT_HELP ) {
	AddFlag( ALib::CommandLineFlag( FLAG_REGEX, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ICASE, false, 0 ) );
}

//---------------------------------------------------------------------------
// Split using bracketed exressions then recall and insert into row.
//---------------------------------------------------------------------------

void SplitRegex :: Split( CSVRow & row ) {
    CSVRow tmp;
    string target = Field() < row.size() ? row[ Field() ] : "";
    mRegex.FindIn( target );
    for( unsigned int i = 0; i < mRegex.SavedMatchCount(); i++ ) {
        tmp.push_back( mRegex.SavedMatch(i) );
    }
    Insert( row, tmp );
}

//---------------------------------------------------------------------------
// Perform regex split.
//---------------------------------------------------------------------------

int SplitRegex :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	GetCommonFlags( cmd );

    string rs = cmd.GetValue( FLAG_REGEX );
    bool ic = cmd.HasFlag( FLAG_ICASE );
    ALib::RegEx::CaseSense cs = ic  ? ALib::RegEx::Insensitive
                                    : ALib::RegEx::Sensitive;
    mRegex = ALib::RegEx( rs, cs );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( Skip( io, row ) ) {
			continue;
		}
		if( ! Pass( io,row ) ) {
            Split( row );
		}
		io.WriteRow( row );
	}

	return 0;
}


//------------------------------------------------------------------------
// Split by fixed positions in field
//---------------------------------------------------------------------------

SplitFixed :: SplitFixed( const string & name,
							const string & desc )
	: SplitBase( name, desc, FSPLIT_HELP ) {
	AddFlag( ALib::CommandLineFlag( FLAG_POS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FLEN, false, 1 ) );
}


//---------------------------------------------------------------------------
// Create list of field lengths. One field is allowed to be of variable
// length, which for each record will evaluate to the length of the input
// minus the lengths of all the fixed length fields.
//---------------------------------------------------------------------------

const string VARLEN = "*";   // variable length

void SplitFixed :: CreateLengths( const string & ls ) {
    ALib::CommaList cl( ls );
    bool havevar = false;
    for( unsigned int i = 0; i < cl.Size(); i++ ) {
        string len = cl.At( i );
        if ( len == VARLEN ) {
            if ( havevar ) {
                CSVTHROW( "Only a single variable-length field is allowed" );
            }
            havevar = true;
            mLengths.push_back( 0 );    // variable length indicator
        }
        else if ( ALib::IsInteger( len ) ) {
            int l = ALib::ToInteger( len );
            if ( l <= 0 ) {
                CSVTHROW( "Invalid field length: " << len );
            }
            mLengths.push_back( l );
        }
        else {
            CSVTHROW( "Invalid field length: " << len );
        }
    }
}

//---------------------------------------------------------------------------
// Split using field lengths. If a variable length field (length 0) was
// specified, its length is the length of the input minus the lengths of all 
// the other fields.
//---------------------------------------------------------------------------

void SplitFixed :: SplitLengths( CSVRow & row ) {
    string target = Field() < row.size() ? row[ Field() ] : "";
    int flens =0;
    for (unsigned int i = 0; i < mLengths.size(); i++) {
        flens += mLengths[i];
    }
    int vlen = target.size() - flens;
    if ( vlen < 0 ) {
        vlen = 0;
    }
    CSVRow tmp;
    int pos = 0;
    for (unsigned int i = 0; i < mLengths.size(); i++) {
        int len = mLengths[i];
        if ( len == 0 ) {
            tmp.push_back( vlen == 0 ? "" : target.substr( pos, vlen ));
            pos += vlen;
        }
        else {
            tmp.push_back( target.substr( pos, len ) );
            pos += len;
        }
    }
    Insert( row, tmp );
}

//---------------------------------------------------------------------------
// Get fixed positions then split all inputs
//---------------------------------------------------------------------------

int SplitFixed :: Execute( ALib::CommandLine & cmd ) {

    NotBoth( cmd, FLAG_POS, FLAG_FLEN );
	GetSkipOptions( cmd );
	GetCommonFlags( cmd );

	if ( cmd.HasFlag( FLAG_POS ) ) {
        string pos = cmd.GetValue( FLAG_POS );
        if ( ALib::IsEmpty( pos ) ) {
            CSVTHROW( "Need list of position pairs specified by " << FLAG_POS );
        }
        CreatePositions( pos );
    }
    else if ( cmd.HasFlag( FLAG_FLEN ) ) {
        string lens = cmd.GetValue( FLAG_FLEN );
        if ( ALib::IsEmpty( lens ) ) {
            CSVTHROW( "Need list of lengths specified by " << FLAG_FLEN );
        }
        CreateLengths( lens );
    }
    else {
        CSVTHROW( "Need one of" << FLAG_POS << " or " << FLAG_FLEN );
    }

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( Skip( io, row ) ) {
			continue;
		}
		if( ! Pass( io,row ) ) {
            if ( mLengths.size() ) {
                SplitLengths( row );
            }
            else {
                Split( row );
            }
		}
		io.WriteRow( row );
	}
	return 0;
}

//---------------------------------------------------------------------------
// Split based on fixed positions.
//---------------------------------------------------------------------------

void SplitFixed :: Split( CSVRow & row ) {
	CSVRow tmp;
	string target = Field() < row.size() ? row[ Field() ] : "";
	for ( unsigned int i = 0; i < mPositions.size(); i++ ) {
		unsigned int start = mPositions[i].first;
		unsigned int len = mPositions[i].second;
		if ( start > target.size() ) {
			tmp.push_back( "" );
		}
		else {
			tmp.push_back( target.substr( start, len ) );
		}
	}
	Insert( row, tmp );
}

//---------------------------------------------------------------------------
// Create pairs describing position in a field. Each pair is a start
// position and a length, separated by a colon.
//---------------------------------------------------------------------------

void SplitFixed :: CreatePositions( const string & ps ) {
	ALib::CommaList cl( ps );
	for ( unsigned int i = 0; i < cl.Size(); i++ ) {
		vector <string> tmp;
		if ( ALib::Split( cl.At(i), ':', tmp ) != 2 ) {
			CSVTHROW( "Invalid position " << cl.At(i) );
		}
		AddPosition( tmp[0], tmp[1] );
	}
}

//---------------------------------------------------------------------------
// Validate position and add to list
//---------------------------------------------------------------------------

void SplitFixed :: AddPosition( const string & spos,
									const string & slen ) {
	if ( ! ALib::IsInteger( spos ) || ! ALib::IsInteger( slen ) ) {
		CSVTHROW( "Invalid position " << spos << ":" << slen );
	}

	int pos = ALib::ToInteger( spos );
	int len = ALib::ToInteger( slen);

	if ( pos <= 0 || len <= 0 ) {
		CSVTHROW( "Invalid position " << spos << ":" << slen );
	}

	mPositions.push_back(
		std::make_pair( (unsigned int) pos - 1, (unsigned int) len )
	);
}

//---------------------------------------------------------------------------
// Split field at character value. Default is to split on spaces
//---------------------------------------------------------------------------

SplitChar :: SplitChar( const string & name, const string & desc )
	: SplitBase( name, desc, CSPLIT_HELP ), mTrans( stNone ) {
	AddFlag( ALib::CommandLineFlag( FLAG_CHAR, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_TRANA2N, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_TRANN2A, false, 0 ) );
}

//---------------------------------------------------------------------------
// See if we are splitting on characters or transitions then do split.
//---------------------------------------------------------------------------

int SplitChar :: Execute( ALib::CommandLine & cmd ) {

	GetSkipOptions( cmd );
	GetCommonFlags( cmd );

	if ( cmd.HasFlag( FLAG_TRANA2N ) || cmd.HasFlag( FLAG_TRANN2A ) ) {
		if ( cmd.HasFlag( FLAG_TRANA2N ) && cmd.HasFlag( FLAG_TRANN2A ) ) {
			CSVTHROW( "Only one of " << FLAG_TRANA2N << " or "
							<< FLAG_TRANN2A << " allowed" );
		}
		mTrans = cmd.HasFlag( FLAG_TRANA2N ) ? stAlpha2Num : stNum2Alpha;
		if ( cmd.HasFlag( FLAG_CHAR ) ) {
			CSVTHROW( "Cannot specify both character and transiton ");
		}
	}
	else {
		string sc = cmd.GetValue( FLAG_CHAR, " " );
		if ( sc == "" ) {
			CSVTHROW( "Need characters specified by " << FLAG_CHAR );
		}
		mChars = ALib::UnEscape( sc );
	}

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {
		if ( Skip( io, row ) ) {
			continue;
		}

		if ( ! Pass( io, row ) ) {
			if ( mTrans == stNone ) {
				Split( row );
			}
			else {
				TransSplit( row );
			}
		}
		io.WriteRow( row );
	}

	return 0;
}

//---------------------------------------------------------------------------
// Perform split on characters (pre CSVfix 0.9 only single char was allowed)
// specified with -c flag.
//
// Note we call different overloads of ALib::Split depending on whether
// we have a single character or a string to avoid backward compatibility
// problems.
//---------------------------------------------------------------------------

void SplitChar :: Split( CSVRow & row ) {
	string target = Field() < row.size() ? row[ Field() ] : "";
	CSVRow tmp;
	if ( mChars.size() == 1 ) {
		ALib::Split( target, mChars[0], tmp );
	}
	else {
		ALib::Split( target, mChars, tmp );
	}
	Insert( row, tmp );
}

//----------------------------------------------------------------------------
// split fields on a transition for a alpha to numeric or vice versa.
//----------------------------------------------------------------------------

void SplitChar :: TransSplit( CSVRow & row ) {
	string target = Field() < row.size() ? row[ Field() ] : "";
	char last = 0;		// neither alpha nor digit
	size_t i = 0;
	while( i < target.size() ) {
		char c = target[i];
		if (
			( mTrans == stAlpha2Num && std::isdigit( c ) && std::isalpha( last ) )
			||
			( mTrans == stNum2Alpha && std::isalpha( c ) && std::isdigit( last ) )
		) {
			CSVRow tmp;
			tmp.push_back( target.substr( 0, i ) );
			tmp.push_back( target.substr( i ) );
			Insert( row, tmp );
			return;
		}
		last = c;
		i++;
	}
}

//----------------------------------------------------------------------------

} // end namespace

// end

