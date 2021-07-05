//---------------------------------------------------------------------------
// a_csv.h
//
// CSV handling for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_CSV_H
#define INC_A_CSV_H

#include "a_base.h"
#include <string>
#include <vector>
#include <map>

namespace ALib {

//---------------------------------------------------------------------------
// Parser for single lines of CSV data
//---------------------------------------------------------------------------

class CSVLineParser {

	public:

		CSVLineParser( char sep = ',' );
		~CSVLineParser();

		void Parse( const std::string & csv,
					std::vector <std::string> & data );

		static bool IsValidSep( char c );
		char Separator() const {
			return mSep;
		}

	private:

		char Next();
		char Peek() const;
		std::string GetQuoted();
		std::string GetNonQuoted();

		const std::string * mCSV;		// current input
		unsigned int mPos;				// position in input
		bool mMore;						// are more fields expected?
		char mSep;						// field separator
};


//---------------------------------------------------------------------------
// Base for stream and file parsers
//---------------------------------------------------------------------------

class CSVParser {

	public:

		CSVParser( char sep = ',' );
		virtual ~CSVParser();

		virtual bool ParseNext( std::vector <std::string> & data ) = 0;

	protected:

		CSVLineParser & LineParser() {
			return mLineParser;
		}

	private:

		CSVLineParser mLineParser;
};

//---------------------------------------------------------------------------
// Parse CSV input from stream
//---------------------------------------------------------------------------

class CSVStreamParser : public CSVParser {

	CANNOT_COPY( CSVStreamParser );

	public:

		CSVStreamParser( std::istream & is,
							bool igblank = false,
							bool skipcols = false,
							bool makecolmap = false,
							char csvsep = ',' );
		bool ParseNext( std::vector <std::string> & data );
		unsigned int LineNo() const;
		std::string RawLine() const;

		unsigned int ColIndexFromName(const std::string & name ) const;

	private:

		bool ProcessChar( char c, std::string & line );
		void MakeColMap( const std::string & cols );

		enum State { InVal, InQVal, HaveQ, OutVal };

		State mState;
		std::istream * mStream;
		unsigned int mLineNo;
		std::string mRawLine;
		bool mIgnoreBlankLines;
		bool mSkipColumnNames;
		bool mMakeColMap;

		typedef std::map <std::string, int> ColNameMapType;
		ColNameMapType mColMap;
};


//---------------------------------------------------------------------------
// Parse from named file
//---------------------------------------------------------------------------


class CSVFileParser : public CSVParser {

	CANNOT_COPY( CSVFileParser );

	public:

		CSVFileParser( const std::string & is,
						bool igblank = false,
						char csvsep = ','  );
		virtual ~CSVFileParser();

		bool ParseNext( std::vector <std::string> & data );

		std::string FileName() const {
			return mFileName;
		}

		unsigned int LineNo() const;
		std::string RawLine() const;

	private:

		std::ifstream * mIfstream;
		CSVStreamParser * mParser;
		std::string mFileName;

};

//---------------------------------------------------------------------------

}		// end namespace ALib

#endif

