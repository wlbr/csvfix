//---------------------------------------------------------------------------
// csved_squash.h
//
// squash duplicate rows to single row
//
// Copyright (C) 2014 Neil Butterworth
//---------------------------------------------------------------------------


#include "csved_squash.h"
#include "csved_cli.h"
#include "csved_strings.h"

#include <climits>
#include <cfloat>

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Derived types for integer and double fields
//---------------------------------------------------------------------------


class IntSquashValues : public SquashValues {

    public:

        IntSquashValues( const std::string & nn ) : mNotNum( INT_MAX ) {
            if ( nn != "" ) {
                if ( ! ALib::IsInteger( nn ) ) {
                    CSVTHROW( "Need integer value for " << FLAG_NONUM );
                }
                mNotNum = ALib::ToInteger( nn );
            }
        }

        virtual void Add( unsigned int idx, const std::string & val ) {
            int n;
            if ( ! ALib::IsInteger( val ) ) {
                if ( mNotNum == INT_MAX ) {
                    CSVTHROW( "Non-integer value " << val << " for numeric field" );
                }
                n = mNotNum;
            }
            else {
                n = ALib::ToInteger( val );
            }

            if ( idx >= mValues.size() ) {
                mValues.push_back( n );
            }
            else {
                mValues[idx] += n;
            }
        }

        virtual void AppendTo( CSVRow & row ) {
            for ( auto n : mValues ) {
                row.push_back( ALib::Str( n ) );
            }
        }


    private:

        int mNotNum;
        std::vector <int> mValues;
};

class RealSquashValues : public SquashValues {

    public:

        RealSquashValues( const std::string & nn ) : mNotNum( DBL_MAX ) {
            if ( nn != "" ) {
                if ( ! ALib::IsNumber( nn ) ) {
                    CSVTHROW( "Need real number value for " << FLAG_NONUM );
                }
                mNotNum = ALib::ToReal( nn );
            }
        }

        virtual void Add( unsigned int idx, const std::string & val ) {
            double n;
            if ( ! ALib::IsNumber( val ) ) {
                if ( mNotNum == DBL_MAX ) {
                    CSVTHROW( "Non-real number value " << val << " for numeric field" );
                }
                n = mNotNum;
            }
            else {
                n = ALib::ToReal( val );
            }
            if ( idx >= mValues.size() ) {
                mValues.push_back( n );
            }
            else {
                mValues[idx] += n;
            }
        }

        virtual void AppendTo( CSVRow & row ) {
            for ( auto n : mValues ) {
                row.push_back( ALib::Str( n ) );
            }
        }

    private:

        double mNotNum;
        std::vector <double> mValues;
};

//---------------------------------------------------------------------------
// Register squash command
//---------------------------------------------------------------------------

static RegisterCommand <SquashCommand> rc1_(
	CMD_SQUASH,
	"squash duplicate rows to single row"
);

//----------------------------------------------------------------------------
// Squash command help
//----------------------------------------------------------------------------

const char * const UNIQUE_HELP = {
	"squash duplicate rows to single row \n"
	"usage: csvfix squash [flags] [files ...]\n"
	"where flags are:\n"
	"  -f fields\tfields specifying key (required)\n"
	"  -n fields\toutput only duplicate rows (required)\n"
	"  -nn val\tvalue to use if -n field contains non-numeric value\n"
	"  -rn \t\ttreat -n fields as real numbers rather than as integers\n"
	"#SMQ,SEP,IBL,IFN,OFL,SKIP"

};

//---------------------------------------------------------------------------
// Standard ctor
//---------------------------------------------------------------------------

SquashCommand :: SquashCommand( const string & name,
								const string & desc )
		: Command( name, desc, UNIQUE_HELP), mRealNums( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_NUM, true, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_NONUM, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_REALS, false, 0 ) );

}

//---------------------------------------------------------------------------
// Read inputs and squash rows with same key.
//---------------------------------------------------------------------------

int SquashCommand :: Execute( ALib::CommandLine & cmd ) {

    GetSkipOptions( cmd );
    ProcessFlags( cmd );

	IOManager io( cmd );
	CSVRow row;

	while( io.ReadCSV( row ) ) {

        if ( Skip( io, row ) ) {
			continue;
		}


        string key = MakeKey( row );
        auto it = mKeyValues.find( key );
        if ( it == mKeyValues.end() ) {

            std::shared_ptr <SquashValues> vp(
                mRealNums ? (SquashValues*) new RealSquashValues( mNotNumVal )
                          : (SquashValues*) new IntSquashValues( mNotNumVal ) );
            Accumulate( vp, row );
            mKeyValues.insert( std::make_pair( key, vp ) );
        }
        else {
            Accumulate( it->second, row );
        }
    }

    for ( auto it : mKeyValues ) {
        CSVRow outrow;
        Key2CSV( it.first, outrow );
        it.second->AppendTo( outrow );
        io.WriteRow( outrow );
    }

    return 0;
}


//---------------------------------------------------------------------------
// Convert null-separated key to CSV
//---------------------------------------------------------------------------

void SquashCommand :: Key2CSV( const std::string & key, CSVRow & row) {
    string f;
    for( unsigned int i =0; i < key.size(); i++ ) {
        if ( key[i] == 0 ) {
            row.push_back( f );
            f = "";
        }
        else {
            f += key[i];
        }
    }
}

//---------------------------------------------------------------------------
// Accumulate values (or initialise, if first row seen)
//---------------------------------------------------------------------------

void SquashCommand :: Accumulate( std::shared_ptr<SquashValues>& vp,
                            const CSVRow & row ) {
    int n = 0;
    for ( auto i : mValFields ) {
        string v = GetField( row, i );
        vp->Add( n, v );
        n++;
    }
}

//---------------------------------------------------------------------------
// Create key from row by concatting key fields separated by a null char.
//---------------------------------------------------------------------------

string SquashCommand :: MakeKey( const CSVRow & row ) const{

    string key;
    for( auto i : mKeyFields ) {
        string f = GetField( row, i );
        key += f;
        key += '\0';
    }
    return key;
}

//---------------------------------------------------------------------------
// Read command line parameters.
//---------------------------------------------------------------------------

void SquashCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

    string f = cmd.GetValue( FLAG_COLS, ""  );
	if ( f == "" ) {
        CSVTHROW( FLAG_COLS << " must specify field list" );
	}
	CommaListToIndex( ALib::CommaList( f ), mKeyFields );

    f = cmd.GetValue( FLAG_NUM, ""  );
    if ( f == "" ) {
        CSVTHROW( FLAG_NUM << " must specify field list" );
    }
    CommaListToIndex( ALib::CommaList( f ), mValFields );

	mRealNums = cmd.HasFlag( FLAG_REALS );
	mNotNumVal = cmd.GetValue( FLAG_NONUM, "" );
}


//------------------------------------------------------------------------

} // end namespace

// end

