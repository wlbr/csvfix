//---------------------------------------------------------------------------
// csved_case.h
//
// Case conversion for CSVED
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_CASE_H
#define INC_CSVED_CASE_H

#include "a_str.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------
// Base class for case conversion commands
//----------------------------------------------------------------------------

class CaseBase : public Command {

	public:

		CaseBase( const std::string & name,
					const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	protected:

		virtual void Transform( std::string & s ) = 0;
		std::string Help() const;
		virtual std::string Explain() const = 0;

	private:

		void ProcessRow( CSVRow & row, std::vector <unsigned int > & ci );

};

//---------------------------------------------------------------------------
// Convert to upper case
//----------------------------------------------------------------------------

class UpperCommand : public CaseBase {

	public:

		UpperCommand( const std::string & name,
						const std::string & desc )
			: CaseBase( name, desc ) {}

	protected:

		std::string Explain() const {
			return "convert alphabetc data to upper case\n"
					"usage: csvfix upper [flags] [files ...]\n";

		}

		void Transform( std::string & s ) {
			s = ALib::Upper( s );
		}

};

//----------------------------------------------------------------------------
// Convert to lowercase
//----------------------------------------------------------------------------

class LowerCommand : public CaseBase {

	public:

		LowerCommand( const std::string & name,
						const std::string & desc )
			: CaseBase( name, desc ) {}

	protected:

		std::string Explain() const {
			return "convert alphabetc data to lower case\n"
					"usage: csvfix lower [flags] [files ...]\n";
		}

		void Transform( std::string & s ) {
			s = ALib::Lower( s );
		}

};


//----------------------------------------------------------------------------
// Convert to mixed (each word capitalised) case
//----------------------------------------------------------------------------

class MixedCommand : public CaseBase {

	public:

		MixedCommand( const std::string & name,
						const std::string & desc )
			: CaseBase( name, desc ) {}

	protected:

		std::string Explain() const {
			return "convert alphabetc data to mixed (capilalised)  case\n"
					"usage: csvfix mixed [flags] [files ...]\n";
		}

		void Transform( std::string & s ) {
			s = ALib::Capitals( s );
		}

};

//---------------------------------------------------------------------------

}	// end namespace


#endif

