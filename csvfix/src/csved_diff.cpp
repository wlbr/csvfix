//---------------------------------------------------------------------------
// csved_diff.cpp
//
// diff two csv files.
//
// This code is adapted from that presented in this article:
//
//    http://www.codeproject.com/KB/recipes/diffengine.aspx
//
// I'm still in the process of converting it, but the current code
// does work. Lack of comments is my fault.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_cli.h"
#include "csved_diff.h"
#include "csved_strings.h"

#include <string>
#include <vector>
#include <memory>
using std::string;

namespace CSVED {

const int BAD_INDEX = -1;

//----------------------------------------------------------------------------

class CSVList {

	public:

		CSVList() {
		}

		unsigned int Count() const {
			return mRows.size();
		}

		void Add( const CSVRow  & row ) {
				mRows.push_back( row );
		}

		const CSVRow & At( unsigned int i ) const {
			return mRows.at( i );
		}

	private:

		std::vector <CSVRow> mRows;

};

//----------------------------------------------------------------------------

enum EditAction { eaNoChange, eaReplace, eaDelSrc, eaAddDest };

struct ResultSpan {

	public:

		ResultSpan( EditAction ea, int di, int si, int len )
			: mAction( ea ), mDestIndex( di ), mSrcIndex( si ), mLen( len ) {
		}

		int IncLen( int n ) {
			return mLen += n;
		}

		int Cmp( const ResultSpan & r ) const {
			if ( mDestIndex < r.mDestIndex ) {
				return -1;
			}
			else if ( mDestIndex == r.mDestIndex ) {
				return 0;
			}
			else {
				return 1;
			}
		}

		bool operator < ( const ResultSpan & r ) const {
			return Cmp( r ) < 0;
		}

		void DumpOn( std::ostream & os ) const {
			os << "action: " << mAction
			   << "\ndest: " << mDestIndex
			   << "\nsrc:  " << mSrcIndex
			   << "\nlen:  " << mLen
			   << std::endl;
		}

		EditAction mAction;
		int mDestIndex, mSrcIndex, mLen;

};

typedef std::vector <ResultSpan> Results;

//----------------------------------------------------------------------------

enum DiffStatus {
		dsMatched = 1,
		dsNoMatch = -1,
		dsUnknown = -2
};


class State {

	public:

		State() : mStart( BAD_INDEX ), mLen( dsUnknown ) {
		}

		int Start() const { return mStart; }
		int End() const { return mStart + mLen - 1; }
		int Length() const {
			if ( mLen > 0 ) {
				return mLen;
			}
			else {
				return mLen == 0 ? 1 : 0;
			}
		}

		DiffStatus Status() const {
			if ( mLen > 0 ) {
				return dsMatched;
			}
			else if ( mLen == -1 ) {
				return dsNoMatch;
			}
			else {
				return dsUnknown;
			}
		}

		void SetMatch( int start, int len ) {
			mStart = start;
			mLen = len;
		}

		void SetNoMatch() {
			mStart = BAD_INDEX;
			mLen = dsNoMatch;
		}

		bool HasValidLength( int start, int end, int maxposs ) {
			if ( mLen > 0) {
				if ( maxposs < mLen || mStart  < start || End() > end ) {
					* this = State(); // reset
				}
			}
			return ( mLen != dsUnknown );
		}

	private:

		int mStart, mLen;

};

typedef std::vector <State> StateList;

//----------------------------------------------------------------------------

class Differ {

	public:

		Differ( const DiffCommand * cmd );
		Results Diff( const CSVList & src, const CSVList & dest );
		void Display( const Results & r, IOManager & io ) const;

		static bool Same( const Results & r );

	private:


		void ProcessRange( int dstart, int dend, int sstart, int send );
		int SourceMatchLen( int di, int si, int maxlen ) const;
		void LongestSourceMatch( State & curitem, int di,int dend, int sstart,int send) const;
		bool AddChanges( Results & r, int dest, int nextdest, int src, int nextsrc );
		Results MakeReport();

		bool NotEq( const CSVRow & src, const CSVRow & dest ) const;

		const CSVList * mSrc;
		const CSVList * mDest;
		Results mMatches;
		StateList mStates;
		const DiffCommand * mCmd;
};


//---------------------------------------------------------------------------
// Register diff  command
//---------------------------------------------------------------------------

static RegisterCommand <DiffCommand> rc1_(
	CMD_DIFF,
	"compare two CSV files"
);


//----------------------------------------------------------------------------
// Help
//----------------------------------------------------------------------------

const char * const DIFF_HELP = {
	"compare two CSV files and display differences\n"
	"usage: csvfix diff [flags] file1 file2\n"
	"where flags are:\n"
	"  -f fields\tfields to check for differences (default all)\n"
	"  -q\t\tdo not report, only return same/different status\n"
	"  -ic\t\tignore case when diffing\n"
	"  -is\t\tignore leading and trailing spaces when diffing\n"
	"#ALL"
};

//----------------------------------------------------------------------------
// The diff command
//----------------------------------------------------------------------------

DiffCommand :: DiffCommand( const string & name, const string & desc )
				: Command( name, desc, DIFF_HELP ),
				  mReport( true ), mTrim( false ), mIgnoreCase( false ) {

	AddFlag( ALib::CommandLineFlag( FLAG_COLS, false, 1 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_QUIET, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ICASE, false, 0 ) );
	AddFlag( ALib::CommandLineFlag( FLAG_ISPACE, false, 0 ) );


}

//----------------------------------------------------------------------------
// Helper to read the indexed input stream into a CSVList
//----------------------------------------------------------------------------

static void ReadCSV( IOManager & io, int index, CSVList & csvlist ) {

	std::unique_ptr<ALib::CSVStreamParser> p( io.CreateStreamParser( index ) );

	CSVRow row;
	while( p->ParseNext( row ) ) {
		csvlist.Add( row );
	}
}

//----------------------------------------------------------------------------
// Read the two input files (source and destination in the parlance of the
// adapted code) and produce a diff. Diff format is currently of my
// own devising!
//----------------------------------------------------------------------------

int DiffCommand :: Execute( ALib::CommandLine & cmd ) {

	ProcessFlags( cmd );
	IOManager io( cmd );
	if ( io.InStreamCount() != 2 ) {
		CSVTHROW( "diff needs two input files" );
	}

	CSVList src, dest;
	ReadCSV( io, 0, src );
	ReadCSV( io, 1, dest );

	Differ differ( this );

	const Results & r = differ.Diff( src, dest );

	if ( mReport ) {
		differ.Display( r, io );
	}

	return Differ::Same( r ) ? 0 : 1;
}

//----------------------------------------------------------------------------
// Get command line options
//----------------------------------------------------------------------------

void DiffCommand :: ProcessFlags( ALib::CommandLine & cmd ) {

	mReport = ! cmd.HasFlag( FLAG_QUIET );
	mIgnoreCase = cmd.HasFlag( FLAG_ICASE );
	mTrim = cmd.HasFlag( FLAG_ISPACE );
	if ( cmd.HasFlag( FLAG_COLS ) ) {
		ALib::CommaList cl( cmd.GetValue( FLAG_COLS ) );
		CommaListToIndex( cl, mFields );
	}
}


//----------------------------------------------------------------------------

Differ :: Differ ( const DiffCommand * cmd ) : mSrc( 0 ), mDest( 0 ), mCmd( cmd ) {
}


Results Differ :: Diff( const CSVList & src, const CSVList & dest ) {

	mSrc = & src;
	mDest = & dest;
	mMatches = Results();
	Results r;

	int sz = dest.Count();
	if ( sz && src.Count() ) {
		mStates = StateList( sz );
		ProcessRange( 0, sz - 1, 0, src.Count() - 1 );
		r = MakeReport();
	}
	return r;
}


//----------------------------------------------------------------------------
// Helper to compare strings possibly uppercased and trimmed
//----------------------------------------------------------------------------

static bool Cmp( const string & src, const string & dest, bool ic, bool is ) {
	string s = src, d = dest;
	if ( ic ) {
		s = ALib::Upper( s );
		d = ALib::Upper( d );
	}
	if ( is ) {
		s = ALib::Trim( s );
		d = ALib::Trim( d );
	}
	return s != d;
}

//----------------------------------------------------------------------------
// Helper to do comparison possibly using user supplied field list
//----------------------------------------------------------------------------

bool Differ :: NotEq( const CSVRow & src, const CSVRow & dest ) const {
	if ( mCmd->mFields.size() ) {
		for ( unsigned int  i = 0; i < mCmd->mFields.size(); i++ ) {
			string ss = GetField( src,  mCmd->mFields[i]  );
			string ds = GetField( dest,  mCmd->mFields[i]  );
			if ( Cmp( ss, ds, mCmd->mIgnoreCase, mCmd->mTrim )) {
				return true;
			}
		}
		return false;
	}
	else {
		unsigned int sz = std::max( src.size(), dest.size() );
		for ( unsigned int  i = 0; i < sz; i++ ) {
			string ss = GetField( src,  i  );
			string ds = GetField( dest,  i  );
			if ( Cmp( ss, ds, mCmd->mIgnoreCase, mCmd->mTrim )) {
				return true;
			}
		}
		return false;
	}
}


//----------------------------------------------------------------------------

int Differ :: SourceMatchLen( int di, int si, int maxlen ) const {
	int matchcount = 0;
	for ( ; matchcount < maxlen; matchcount++ ) {
		if ( NotEq( mDest->At( di + matchcount), mSrc->At( si + matchcount ) ) ) {
			break;
		}
	}
	return matchcount;
}

void Differ :: LongestSourceMatch( State & curitem, int di,int dend,
										int sstart,int send) const {

	int maxdestlen = (dend - di) + 1;
	int bestlen = 0;
	int besti = -1;

	for ( int si = sstart; si <= send; si++ ) {
		int maxlen = std::min(maxdestlen,(send - si) + 1);
		if ( maxlen <= bestlen ) {
			//No chance to find a longer one any more
			break;
		}
		int curlen = SourceMatchLen( di, si, maxlen );
		if ( curlen > bestlen ) {
			//This is the best match so far
			besti = si;
			bestlen = curlen;
		}
		//jump over the match
		si += bestlen;
	}

	//DiffState cur = _stateList.GetByIndex(destIndex);
	if ( besti == -1) {
		curitem.SetNoMatch();
	}
	else {
		curitem.SetMatch( besti, bestlen);
	}
}

void Differ :: ProcessRange(int dstart, int dend, int sstart, int send)  {

	State curstate, beststate;

	int bestlen = -1, bestindex = -1;
	for ( int di = dstart; di <= dend; di++ ) {
		int maxposlen = (dend - di) + 1;
		if ( maxposlen <= bestlen )	{
			//we won't find a longer one even if we looked
			break;
		}

		curstate = mStates.at( di );		// ***** PROBLEMO ******

		if ( ! curstate.HasValidLength(sstart , send, maxposlen ) ) {
			//recalc new best length since it isn't valid or has never been done.
			LongestSourceMatch( curstate, di, dend, sstart, send);
		}

		if ( curstate.Status() == dsMatched ) {
			if ( curstate.Length() > bestlen ) {
				//this is longest match so far
				bestindex = di;
				bestlen = curstate.Length();
				beststate = curstate;
			}
		}
	}

	if ( bestindex >=  0 ) {
		int si = beststate.Start();
		mMatches.push_back( ResultSpan( eaNoChange, bestindex, si ,bestlen));
		if ( dstart < bestindex ) {
			//Still have more lower destination data
			if ( sstart < si ) 	{
				//Still have more lower source data
				// Recursive call to process lower indexes
				ProcessRange( dstart, bestindex - 1, sstart, si - 1 );
			}
		}
		int udstart = bestindex + bestlen;
		int usstart = si + bestlen;
		if ( dend > udstart ) 		{
			//we still have more upper dest data
			if ( send > udstart ) {
				//set still have more upper source data
				// Recursive call to process upper indexes
				ProcessRange( udstart, dend, usstart, send );
			}
		}
	}
}

bool Differ :: AddChanges( Results & report, int dest, int nextdest,
								int src, int nextsrc ) {

	bool retval = false;
	int diffdest = nextdest - dest;
	int diffsrc =  nextsrc - src;
	if ( diffdest > 0) {
		if ( diffsrc > 0)
		{
			int mindiff = std::min( diffdest, diffsrc );
			report.push_back( ResultSpan( eaReplace, dest, src, mindiff ) );
			if ( diffdest > diffsrc ) {
				dest += mindiff;
				report.push_back( ResultSpan( eaAddDest, dest, -1, diffdest - diffsrc ) );
			}
			else {
				if ( diffsrc > diffdest ) {
					src += mindiff;
					report.push_back( ResultSpan( eaDelSrc, -1, src, diffsrc - diffdest ) );
				}
			}
		}
		else {
			report.push_back( ResultSpan( eaAddDest, dest, -1, diffdest ) );
		}
		retval = true;
	}
	else {
		if ( diffsrc > 0 )
		{
			report.push_back( ResultSpan( eaDelSrc, -1, src ,diffsrc ) );
			retval = true;
		}
	}
	return retval;
}


Results Differ ::  MakeReport()	{

	Results res;
	int dcount = mDest->Count();
	int scount = mSrc->Count();

	//Deal with the special case of empty files
	if ( dcount == 0 )	{
		if ( scount > 0 )		{
			res.push_back( ResultSpan( eaDelSrc, -1, 0,scount ) );
		}
		return res;
	}
	else {
		if ( scount == 0 ) {
			res.push_back( ResultSpan( eaAddDest, 0, -1, dcount ) );
			return res;
		}
	}

	std::sort( mMatches.begin(), mMatches.end() );

	int dest = 0;
	int src = 0;
	ResultSpan * last = 0;

	for ( unsigned int i = 0; i < mMatches.size(); i++ ) {
		ResultSpan & drs = mMatches[i];
		if ( (! AddChanges( res, dest, drs.mDestIndex, src, drs.mSrcIndex ))
				&& last != 0 ) {
			last->IncLen( drs.mLen );
		}
		else {
			res.push_back( drs );
		}
		dest = drs.mDestIndex + drs.mLen;
		src = drs.mSrcIndex + drs.mLen;
		last = & drs;
	}
	AddChanges( res, dest, dcount, src, scount);

	return res;
}

static string Indicator( const string & ic, int index ) {
	return "\"" + ic + "\",\"" + ALib::Str( index + 1 ) + "\",";

}

void Differ :: Display( const Results & r, IOManager & io ) const {
	for ( unsigned int i = 0; i < r.size(); i++ ) {
		const ResultSpan & rs = r[i];
		if ( rs.mAction == eaNoChange ) {
			for ( int i = 0; i < rs.mLen; i++ ) {
			}

		}
		else if ( rs.mAction == eaAddDest ) {
			for ( int i = 0; i < rs.mLen; i++ ) {
				io.Out() << Indicator( "+", rs.mDestIndex + i );
				io.WriteRow( mDest->At( rs.mDestIndex + i ) );
			}
		}
		else if ( rs.mAction == eaDelSrc ) {
			for ( int i = 0; i < rs.mLen; i++ ) {
				io.Out() << Indicator( "-", rs.mSrcIndex + i );
				io.WriteRow( mSrc->At( rs.mSrcIndex + i ) );
			}
		}
		else if ( rs.mAction == eaReplace ) {
			for ( int i = 0; i < rs.mLen; i++ ) {
				io.Out() << Indicator( "-", rs.mSrcIndex + i );
				io.WriteRow( mSrc->At( rs.mSrcIndex + i ) );
				io.Out() << Indicator( "+", rs.mDestIndex + i );
				io.WriteRow( mDest->At( rs.mDestIndex + i ) );
			}
		}
		else {
			CSVTHROW( "Bad action code in Differ::Display" );
		}
	}
}

bool Differ :: Same( const Results & r ) {
	return r.size() == 1 && r[0].mAction == eaNoChange;
}

//----------------------------------------------------------------------------

} // namespace





