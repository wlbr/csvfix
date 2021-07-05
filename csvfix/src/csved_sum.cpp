//---------------------------------------------------------------------------
// csved_sum.cpp
//
// create summary info from CSV input
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_rand.h"
#include "csved_cli.h"
#include "csved_sum.h"
#include "csved_strings.h"
#include <algorithm>

using std::string;
using std::vector;

namespace CSVED {

//----------------------------------------------------------------------------
// Register summary command
//----------------------------------------------------------------------------

static RegisterCommand <SummaryCommand> rc1_(
	CMD_SUMMARY,
	"summarise CSV data"
);

//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const SUM_HELP = {
	"provide summarisation functions\n"
	"usage: csvfix summary [flags] [files ...]\n"
	"where flags are:\n"
	"  -avg fields\tcalculate numeric average of specified fields\n"
	"  -frq fields\tprepend frequencies of specified fields to output\n"
	"  -max fields\tfind and display maximum of fields\n"
	"  -min fields\tfind and display minimum of fields\n"
	"  -med fields\tcalculate median of fields\n"
	"  -mod fields\tcalculate mode of fields\n"
	"  -sum fields\tperform summation of fields\n"
	"  -siz\t\tfind max and min lengths of all fields\n"
	"  Note that only one of the above flags can be specified\n"
	"#SMQ,SEP,IBL,IFN,OFL"
};

//----------------------------------------------------------------------------
// Standard command ctor
//----------------------------------------------------------------------------

SummaryCommand :: SummaryCommand( const string & name,
								const string & desc )
		: Command( name, desc, SUM_HELP ) {

	AddFlag( ALib::CommandLineFlag( FLAG_AVG, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_MIN, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_MAX, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_FREQ, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_MEDIAN, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_MODE, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_SUM, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_SIZE, false, 0 ) );
}

//----------------------------------------------------------------------------
// Read all rows and then perform requested summary op on them.
//----------------------------------------------------------------------------

int SummaryCommand :: Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );
	IOManager io( cmd );

	CSVRow row;

	SizeMap sizemap;
	while( io.ReadCSV( row ) ) {
		if ( mType == Size ) {
			RecordSizes( row, sizemap );
		}
		else {
			mRows.push_back( row );
		}
	}


	if ( mType == Size ) {
		PrintSizes( io, sizemap );
	}
	else {
		if ( mRows.size() == 0 ) {
			CSVTHROW( "No input" );
		}
		Summarise( io );
	}
	return 0;
}

//----------------------------------------------------------------------------
// Find the min and max lengths of each field
//----------------------------------------------------------------------------

void SummaryCommand :: RecordSizes( const CSVRow & row, SizeMap & sm ) {
	for ( unsigned int i = 0; i < row.size(); i++ ) {
		int sz = row[i].size();
		SizeMap::iterator pos = sm.find( i );
		if ( pos == sm.end() ) {
			sm[ i ] = std::make_pair( INT_MAX, 0 );
		}
		sm[i].first = std::min( sm[i].first, sz );
		sm[i].second = std::max( sm[i].second, sz );
	}
}

//----------------------------------------------------------------------------
// Print field index and mn/max lengths
//----------------------------------------------------------------------------

void SummaryCommand :: PrintSizes( IOManager & io, const SizeMap & sm ) {
	SizeMap::const_iterator it = sm.begin();
	while( it != sm.end() ) {
		io.Out() << it->first + 1 << ": "
				 << it->second.first << "," << it->second.second
				 << "\n";
		++it;
	}
}

//----------------------------------------------------------------------------
// Dispatch depending on flag type. Only one flag is allowed.
//----------------------------------------------------------------------------

void SummaryCommand :: Summarise( IOManager & io ) {
	if ( mType == Min || mType == Max ) {
		DoMinMax( io );
	}
	else if ( mType == Sum ) {
		DoSum( io );
	}
	else if ( mType == Average ) {
		DoAvg( io );
	}
	else if ( mType == Frequency ) {
		DoFreq( io );
	}
	else if ( mType == Median ) {
		DoMedian( io );
	}
	else if ( mType == Mode ) {
		DoMode( io );
	}
	else {
		CSVTHROW( "Unsupported option" );
	}
}

//----------------------------------------------------------------------------
// Calculate mode - there may be more than one.
//----------------------------------------------------------------------------

void SummaryCommand :: DoMode( IOManager & io ) {

	unsigned int mode = CalcFreqs();

	for ( FreqMap::const_iterator it = mFreqMap.begin();
			it != mFreqMap.end(); ++it ) {

		if ( it->second.mFreq == mode ) {
			for ( unsigned int i = 0; i < it->second.mIndex.size(); i++ ) {
				CSVRow r = mRows.at( it->second.mIndex.at(i) );
				r.insert( r.begin(), ALib::Str( mode ) );
				io.WriteRow( r );
			}
		}
	}
}


//----------------------------------------------------------------------------
// Calculate frequencies and prepend to existing row data.
//----------------------------------------------------------------------------

void SummaryCommand :: DoFreq( IOManager & io ) {
	CalcFreqs();

	for ( unsigned int i = 0; i < mRows.size(); i++ ) {
		string key = MakeKey( mRows.at(i) );
		unsigned int n = mFreqMap.find( key )->second.mFreq;
		CSVRow r = mRows.at(i);
		r.insert( r.begin(), ALib::Str( n ) );
		io.WriteRow( r );
	}
}

//----------------------------------------------------------------------------
// Calculate frequencies with which specified fields occur. For simplicity
// we use a concatenated key with the parts separated by null characters. This
// could cause problems for CSV data containing nulls - something I have
// never actually encountered. Now also calculates mode.
//----------------------------------------------------------------------------

unsigned int SummaryCommand :: CalcFreqs() {

	mFreqMap.clear();
	unsigned int  mode = 0;

	for ( unsigned int i = 0; i < mRows.size(); i++ ) {
		string key = MakeKey( mRows.at(i) );
		FreqMap::iterator it = mFreqMap.find( key );
		if ( it == mFreqMap.end() ) {
			mFreqMap.insert( std::make_pair( key, FreqMapEntry( i )));
			if ( mode == 0 ) {
				mode = 1;
			}
		}
		else {
			it->second.mFreq++;
			it->second.mIndex.push_back( i );
			if ( mode < it->second.mFreq ) {
				mode = it->second.mFreq ;
			}
		}
	}

	return mode;
}

//----------------------------------------------------------------------------
// Produce concated key for frequency calculation.
//----------------------------------------------------------------------------

string SummaryCommand :: MakeKey( const CSVRow & row ) const {
	string key;
	for ( unsigned int i = 0; i < mFields.size(); i++ ) {
		if ( mFields.at(i) >= row.size() ) {
			key += "";
		}
		else {
			key += row.at( mFields.at(i) );
		}
		key += '\0';
	}
	return key;
}

//----------------------------------------------------------------------------
// Sum specified columns, which must contain numeric values.
//----------------------------------------------------------------------------

void SummaryCommand :: DoSum( IOManager & io ) {
	vector <double> sums;
	SumCols( sums );
	CSVRow r;
	for ( unsigned int i = 0; i < sums.size(); i++ ) {
		r.push_back( ALib::Str( sums.at(i) ) );
	}
	io.WriteRow( r );
}


//----------------------------------------------------------------------------
// Similar to sum, but divide by number of rows to get average.
//----------------------------------------------------------------------------

void SummaryCommand :: DoAvg( IOManager & io ) {
	vector <double> sums;
	SumCols( sums );
	CSVRow r;
	for ( unsigned int i = 0; i < sums.size(); i++ ) {
		r.push_back( ALib::Str( sums.at(i) / mRows.size() ) );
	}
	io.WriteRow( r );
}

//----------------------------------------------------------------------------
// Find the min/max values, and then print all rows that have those values.
//----------------------------------------------------------------------------

void SummaryCommand :: DoMinMax( IOManager & io ) {
	CSVRow r = mRows.at(0);

	for ( unsigned int i = 1; i < mRows.size(); i++ ) {
		if ( mType == Min && Cmp( mRows.at(i), r ) < 0 ) {
			r = mRows.at(i);
		}
		else if ( mType == Max && Cmp( mRows.at(i), r ) > 0 ) {
			r = mRows.at(i);
		}
	}

	for ( unsigned int i = 0; i < mRows.size(); i++ ) {
		if ( Cmp( r, mRows.at(i) ) == 0 ) {
			io.WriteRow( mRows.at(i) );
		}
	}
}

//----------------------------------------------------------------------------
// Helper struct to do numeric sorting on specific column
//----------------------------------------------------------------------------

struct NumSort {

	NumSort( unsigned int col ) : mCol( col ) {}
	bool operator()( const CSVRow & r1, const CSVRow & r2 ) const {
		if ( mCol >= r1.size() || mCol >= r2.size() ) {
			CSVTHROW( "Invalid field index " << (mCol + 1) );
		}
		double d1 = ALib::ToReal( r1[mCol] );
		double d2 = ALib::ToReal( r2[mCol] );
		if ( d1 < d2 ) {
			return true;
		}
		else if ( d1 > d2 ) {
			return false;
		}
		return false;
	}

	unsigned int mCol;
};

//----------------------------------------------------------------------------
// calculate median values for specified fields
//----------------------------------------------------------------------------

void SummaryCommand :: DoMedian( IOManager & io ) {

	CSVRow r;

	for ( unsigned int i = 0; i < mFields.size(); i++ ) {
		unsigned int col = mFields[i];
		NumSort ns( col );
		std::sort( mRows.begin(), mRows.end(), ns );
		unsigned int sz = mRows.size();
		double d;
		if ( sz % 2 ) {
			unsigned int idx = (sz / 2);
			d = ALib::ToReal( mRows.at(idx).at(col) );
		}
		else {
			unsigned int idx = (sz / 2) - 1;
			double d1 = ALib::ToReal( mRows.at(idx).at(col) );
			double d2 = ALib::ToReal( mRows.at(idx+1).at(col) );
			d = (d1 + d2) /2;
		}

		r.push_back( ALib::Str( d ) );
	}

	io.WriteRow( r );
}

//----------------------------------------------------------------------------
// Helper to add a single row with respect to fields to total.
//----------------------------------------------------------------------------

static void SumRow( vector <double> & sums, const CSVRow & row,
						const FieldList & fl ) {

	for ( unsigned int i = 0; i < fl.size(); i++ ) {
		unsigned int fi = fl.at(i);
		if ( fi >= row.size() ) {
			CSVTHROW( "Invalid field index" );
		}
		sums.at(i) += ALib::ToReal( row.at(fi) );
	}
}

//----------------------------------------------------------------------------
// Add up specified columns.
//----------------------------------------------------------------------------

void SummaryCommand :: SumCols( std::vector <double> & sums ) {
	sums.clear();
	for ( unsigned int i = 0; i < mFields.size(); i++ ) {
		sums.push_back( 0.0 );
	}

	for ( unsigned int i = 0; i < mRows.size(); i++ ) {
		SumRow( sums, mRows.at(i), mFields );
	}
}


//----------------------------------------------------------------------------
// Helper to get fields list - all flags need this.
//----------------------------------------------------------------------------

void SummaryCommand :: GetFields( const ALib::CommandLine & cmd,
									const std::string & flag ) {
		ALib::CommaList cl( cmd.GetValue( flag ) );
		CommaListToIndex( cl, mFields );
}


//----------------------------------------------------------------------------
// Helper template to do actual comparison, returning as for strcmp()
//----------------------------------------------------------------------------

template <typename T>
int TCmp( const T & t1, const T & t2 ) {
	if ( t1 < t2 ) {
		return -1;
	}
	else if ( t1 == t2 ) {
		return 0;
	}
	else {
		return 1;
	}
}


//----------------------------------------------------------------------------
// Compare two strings. If both contain numbers, compare numerically.
//----------------------------------------------------------------------------

static int NSCmp( const std::string & s1, const std::string & s2 ) {
	if ( ALib::IsNumber( s1 ) && ALib::IsNumber( s2 ) ) {
		double d1 = ALib::ToReal( s1 );
		double d2 = ALib::ToReal( s2 );
		return TCmp( d1, d2 );
	}
	else {
		return TCmp( s1, s2 );
	}
}

//----------------------------------------------------------------------------
// Compare two rows, with respect to fields specified by user.
//----------------------------------------------------------------------------

int SummaryCommand :: Cmp( const CSVRow & r1, const CSVRow & r2 ) const {
	for ( unsigned int i = 0; i < mFields.size(); i++ ) {
		unsigned int fi = mFields[i];
		if ( fi >= r1.size() || fi >= r2.size() ) {
			CSVTHROW( "Bad field index" );
		}
		int cmp = NSCmp( r1[fi], r2[fi] );

		if ( cmp != 0 ) {
			return cmp;
		}
	}
	return 0;
}


//----------------------------------------------------------------------------
// Handle all user options with error checking
//----------------------------------------------------------------------------

void SummaryCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

	int nf = CountNonGeneric( cmd );
	if ( nf == 0 ) {
		CSVTHROW( "Need a summary flag" );
	}
	else if ( nf != 1 ) {
		CSVTHROW( "Only one summary flag allowed" );
	}

	if ( cmd.HasFlag( FLAG_AVG ) ) {
		mType = Average;
		GetFields( cmd, FLAG_AVG );
	}
	else if ( cmd.HasFlag( FLAG_MIN ) ) {
		mType = Min;
		GetFields( cmd, FLAG_MIN );
	}
	else if ( cmd.HasFlag( FLAG_MAX ) ) {
		mType = Max;
		GetFields( cmd, FLAG_MAX );
	}
	else if ( cmd.HasFlag( FLAG_FREQ ) ) {
		mType = Frequency;
		GetFields( cmd, FLAG_FREQ );
	}
	else if ( cmd.HasFlag( FLAG_MEDIAN ) ) {
		mType = Median;
		GetFields( cmd, FLAG_MEDIAN);
	}
	else if ( cmd.HasFlag( FLAG_MODE) ) {
		mType = Mode;
		GetFields( cmd, FLAG_MODE );
	}
	else if ( cmd.HasFlag( FLAG_SUM ) ) {
		mType = Sum;
		GetFields( cmd, FLAG_SUM );
	}
	else if ( cmd.HasFlag( FLAG_SIZE ) ) {
		mType = Size;
	}
	else {
		CSVTHROW( "Should never happen in SummaryCommand::ProcessFlags" );
	}
}



//----------------------------------------------------------------------------

} // end namespace

// end

