//---------------------------------------------------------------------------
// csved_template.h
//
// templated output command for csved
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_TEMPLATE_H
#define INC_CSVED_TEMPLATE_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class TemplateCommand : public Command {

	public:

		TemplateCommand( const std::string & name,
							const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:


		std::string ReplaceColumns( const std::string & tplate, const CSVRow & row );
		void HandleSpecialChars( const std::string & tplate, char c, unsigned int & pos,
									const CSVRow & row,
									std::string & out );

		std::string Eval( const CSVRow & row, const std::string & expr );
		void ReadTemplate( const ALib::CommandLine & cmd );
		void FileOut( const CSVRow & row );

		std::string mTemplate, mFileTemplate;

};


//------------------------------------------------------------------------

}	// end namespace

#endif

