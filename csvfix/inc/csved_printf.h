//---------------------------------------------------------------------------
// csved_printf.h
//
// printf-style formatting
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_UNIQUE_H
#define INC_CSVED_UNIQUE_H

#include "a_base.h"
#include "csved_command.h"
#include <map>

namespace CSVED {

//---------------------------------------------------------------------------

class PrintfCommand : public Command {

	public:

		PrintfCommand( const std::string & name,
							const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		enum FmtType { None, Literal, Formatter, Ignore };
		struct Format {
			FmtType mType;
			std::string mText;
			Format( FmtType type, const std::string & text )
				: mType( type ), mText( text ) {
			}
		};

		void ParseFormat( const std::string & fmt );
		Format GetFormat( unsigned int & pos, const std::string & fmt );
		Format GetFormatter( unsigned int & pos, const std::string & fmt );
		Format GetLiteral( unsigned int & pos, const std::string & fmt );
		void DumpFmt();
		std::string FormatRow( const CSVRow & row );
		std::string GetField( const CSVRow & row, unsigned int i );

		std::vector <Format> mFmtLine;
		FieldList mOrder;
		bool mCSVQuote;

};

//------------------------------------------------------------------------

}	// end namespace

#endif

