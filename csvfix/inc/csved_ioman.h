//---------------------------------------------------------------------------
// csved_ioman.h
//
// I/O managemant for CSVED
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_IOMAN_H
#define INC_CSVED_IOMAN_H

#include "a_base.h"
#include "a_env.h"
#include "a_csv.h"
#include "csved_util.h"
#include <iostream>

namespace CSVED {

//---------------------------------------------------------------------------
// IOManager supports watchers for I/O events - only one event so far  - the
// opening of a new CSv stream for input.
//----------------------------------------------------------------------------

class IOWatcher {

	public:

		virtual ~IOWatcher();
		virtual void OnNewCSVStream( const std::string & filename,
										const ALib::CSVStreamParser * p ) = 0;
};


//----------------------------------------------------------------------------
// IOManager handles all file opening, readinf & CSV parsing actions
//----------------------------------------------------------------------------

class IOManager {

	CANNOT_COPY( IOManager );

	public:

		IOManager( const ALib::CommandLine & cmdline,
						bool colmap = false, bool preopen = true );
		~IOManager();

		void AddWatcher( IOWatcher & w );

		unsigned int InStreamCount() const;

		std::istream & In( unsigned int index ) const;
		std::string InFileName( unsigned int index ) const;

		std::string CurrentFileName() const;
		unsigned int CurrentLine() const;
		std::string CurrentInput() const;

		bool ReadLine( std::string & line );
		bool ReadCSV( CSVRow & row );
		void WriteRow( const CSVRow & row, bool ignoredq = false );

		std::ostream & Out() const;

		ALib::CSVStreamParser * CreateStreamParser( unsigned int  in );

	private:

		unsigned int  OpenStreams( );
		void OpenInputFile( const std::string & fname );
		void OpenOutputFile( const std::string & fname );
		void ClearStreams();
		void GetGenOpts( const ALib::CommandLine & cl );

		struct Input {
			std::string mFileName;
			std::istream * mStream;

			Input( const std::string & fname, std::istream * is )
				: mFileName( fname ), mStream( is ) {}
		};

		const ALib::CommandLine & mCmdLine;
		unsigned int mInputIndex, mCurrentLine;
		std::vector <Input> mInputs;
		std::ostream * mOutput;
		ALib::CSVStreamParser * mParser;
		std::string mCurrentInput;
		bool mIgnoreBlankLines;
		bool mSmartQuotes;
		bool mSkipColNames;
		bool mMakeColMap;
		std::vector <IOWatcher *> mWatchers;
		char mCSVSep;
		FieldList mQuoteFields;
		bool mRetainSep;
		char mOutputSep;
		bool mPreOpen;
		std::string mHeader;
};


//---------------------------------------------------------------------------

}	// end namespace

#endif
