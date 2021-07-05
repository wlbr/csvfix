//---------------------------------------------------------------------------
// csved_xml.h
//
// read xml tables for csved
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_XML_H
#define INC_CSVED_XML_H

#include "a_base.h"
#include "a_xmlparser.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class ReadXMLCommand : public Command {

	public:

		ReadXMLCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void TableToCSV( const ALib::XMLElement * e,
							IOManager & io , unsigned int idx );
		std::string TDToCSV( const ALib::XMLElement * td );					
		const ALib::XMLElement * FindTable( const ALib::XMLElement * e );
		void WriteRow( const ALib::XMLElement * r, IOManager & io ); 
};


//------------------------------------------------------------------------

}	// end namespace

#endif

