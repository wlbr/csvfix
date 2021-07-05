//----------------------------------------------------------------------------
// a_dir.h
//
// portable directory functions
//
// Copyright (C) 2009 Neil Butterworth
//----------------------------------------------------------------------------

#ifndef INC_A_DIR_H
#define INC_A_DIR_H

#include "a_base.h"


namespace ALib {

//----------------------------------------------------------------------------
// Base class for directory listing entries
//----------------------------------------------------------------------------

class DirListEntry {

	public:

		DirListEntry();
		virtual ~DirListEntry();

		virtual std::string Name() const = 0;
		virtual bool IsDir() const = 0;
		virtual bool IsFile() const = 0;

};

//----------------------------------------------------------------------------
// Directory list represents one level (no tree structure) directory listing
//----------------------------------------------------------------------------

class DirList {

	public:

		DirList( const std::string & fspec = "" );
		virtual ~DirList();

		unsigned int Populate( const std::string & fspec );
		unsigned int Add( const std::string & fspec );
		unsigned int Add( const std::string & fspec, char sep );

		void Clear();

		unsigned int Count() const;
		const DirListEntry * At( unsigned int i ) const;

		static bool DirExists( const std::string & dir );
		static bool FileExists( const std::string & file );

	private:

		std::vector <DirListEntry *> mEntries;
};

//----------------------------------------------------------------------------

} // namespace


#endif
