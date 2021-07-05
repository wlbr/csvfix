//---------------------------------------------------------------------------
// csved_valid.h
//
// validation for CSVED
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_VALID_H
#define INC_CSVED_VALID_H

#include "a_base.h"
#include "csved_command.h"
#include "csved_rules.h"

namespace CSVED {

//---------------------------------------------------------------------------
// Validate input against set of rules.
//---------------------------------------------------------------------------

class ValidateCommand : public Command {

	public:


		ValidateCommand( const std::string & name,
							const std::string & desc );

		~ValidateCommand();

		int Execute( ALib::CommandLine & cmd );

	private:

		void Clear();

		void Report( IOManager & io,
						const ValidationRule::Results & res,
						int errcount  ) const;

		std::string ReadName( const std::string & line,
								unsigned int & pos ) const;

		FieldList ReadFields( const std::string & line,
								unsigned int & pos ) const;

		ValidationRule::Params  ReadParams( const std::string & line,
											unsigned int & pos ) const;

		bool SkipSpaces( const std::string & line, unsigned int & pos ) const;

		void ReadValidationFile( const ALib::CommandLine & cmd );

		void ReadQuotedString( ValidationRule::Params & params,
								const std::string & line,
								unsigned int  & pos ) const;

		void ReadWSTermString( ValidationRule::Params & params,
								const std::string & line,
								unsigned int  & pos ) const;

		void GetOutputMode( const ALib::CommandLine & cl );

		enum OutMode { Reports, Passes, Fails };
		OutMode mOutMode;

		std::vector <ValidationRule *> mRules;

};

//------------------------------------------------------------------------

}	// end namespace

#endif

