//---------------------------------------------------------------------------
// csved_date.h
//
// read & write dates
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_DATE_H
#define INC_CSVED_DATE_H

#include "a_base.h"
#include "a_date.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------
// This class uses a mask in the form d/m/y to read dates from strings.
//---------------------------------------------------------------------------

class MaskedDateReader {

	public:

		MaskedDateReader( const std::string & mask,
							const std::string & months = "",
							unsigned int ybase = 0 );

		void SetMask( const std::string & mask );
		void SetYearBase( unsigned int ybase );
		void SetMonths( const std::string & months );

		bool Read( const std::string & s, ALib::Date & d );

	private:

		char ReadDMY( const std::string & mask, unsigned int & pos );
		std::string ReadSep( const std::string & mask, unsigned int & pos );

		void MakeDay( const std::string & s, int & day  );
		void MakeMonth( const std::string & s, int & day  );
		void MakeYear( const std::string & s, int & day  );

		char mDMY[3];
		std::string mSep[2];
		int mYearBase;
		ALib::CommaList mMonthNames;
};


//---------------------------------------------------------------------------
// Read a date in some user format into ISO yyyy-mm-dd format
//---------------------------------------------------------------------------


class DateReadCommand : public Command {

	public:

		DateReadCommand( const std::string & name,
						const std::string & desc );

		~DateReadCommand();

		int Execute( ALib::CommandLine & cmd );

	private:

		bool ConvertDates( CSVRow & row );
		void ProcessFlags( ALib::CommandLine & cmd );

		FieldList mFields;
		MaskedDateReader * mReader;

		enum WriteAction {
			WriteAll, WriteBad, WriteGood
		};

		WriteAction mWriteAction;
};

//---------------------------------------------------------------------------
// Output formatted dates
//----------------------------------------------------------------------------

class DateFormatCommand : public Command {

	public:

		DateFormatCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void FormatDates( CSVRow & row );
		std::string FormatDate( const std::string & ds );
		void ProcessFlags( ALib::CommandLine & cmd );
		void BuildFormat( const std::string & fmt );

		FieldList mFields;

		struct FmtEntry {
			std::string mText;
			bool mIsFmt;
			FmtEntry( const std::string & txt, bool isfmt )
				: mText( txt ), mIsFmt( isfmt ) {}
		};

		void AddFmt( std::string & s );
		void AddLit( std::string & s );

		std::vector <FmtEntry> mFormat;
};


//------------------------------------------------------------------------

}	// namespace

#endif

