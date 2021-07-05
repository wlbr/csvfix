//---------------------------------------------------------------------------
// csved_fromxml.h
//
// Convert xml file to CSV
//
// Copyright (C) 2010 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_FROMXML_H
#define INC_CSVED_FROMXML_H

#include "a_str.h"
#include "a_xmltree.h"
#include "csved_command.h"
#include "csved_ioman.h"

namespace CSVED {

//----------------------------------------------------------------------------

class FromXMLCommand : public Command {

	public:

		FromXMLCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		enum OutputType { otRecord, otParent };

		void ProcessFlags( const ALib::CommandLine & cmd );
		void ProcessXMLFile( const std::string &  file );
		void GetRecordsFrom( const ALib::XMLElement * e ) ;

		void OutputRecordData( CSVRow & row, const ALib::XMLElement * e, OutputType ot   );
		void OutputAttributes( CSVRow & row, const ALib::XMLElement * e  );
		void OutputParents( CSVRow & row );

		std::string MakePathTo( const ALib::XMLElement * e );

		bool IsRecordPath( const std::string & path );
		bool ShouldBeExcluded( const std::string & path );

		bool IsEmptyLeaf( const ALib::XMLElement * e );
		bool IsOnRecordPath( const ALib::XMLElement * e );

		bool mFromParent, mFromAttrib, mFromKids, mInsertPath;
		IOManager * mIOMan;
		std::vector <const ALib::XMLElement *> mParents;
		std::string mMultiLineSep;
		ALib::CommaList mTagPaths, mExcludePaths;
		std::string mRecordPath;
};

//---------------------------------------------------------------------------

}	// end namespace


#endif
