//---------------------------------------------------------------------------
// a_inifile.h
//
// .INI file stuff for alib. This only supports reading of ini files, not
// maintaining or writing them.
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_INIFILE_H
#define INC_A_INIFILE_H
#include "a_base.h"
#include <map>

namespace ALib {

//---------------------------------------------------------------------------
// Ini file reading class. Does not allow writing of settings.
//----------------------------------------------------------------------------

class IniFile {

	public:

		IniFile( const std::string & filename = "" );
		~IniFile();

		void Clear();

		void Read( const std::string & filename );

		const std::string & FileName() const;

		void Add( const std::string & section,
					const std::string & name,
					const std::string & value );

		bool HasSection( const std::string & section ) const;

		bool HasSetting( const std::string & section,
						 const std::string & name ) const;

		std::string Value( const std::string & section,
							const std::string & name ) const;

	private:

		struct Key {
			std::string mSection;
			std::string mName;

			Key( const std::string & sect, const std::string & name );
			bool operator < ( const Key & e ) const;

		};

		typedef std::map <Key,std::string> MapType;
		MapType mMap;

		std::string mFileName;

};

//----------------------------------------------------------------------------

} // namespace


#endif

