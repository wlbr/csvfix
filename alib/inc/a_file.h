//---------------------------------------------------------------------------
// a_file.h
//
// file utilities for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_FILE_H
#define INC_A_FILE_H

#include "a_base.h"
#include <fstream>

//---------------------------------------------------------------------------

namespace ALib {

//---------------------------------------------------------------------------
// Filename class
//---------------------------------------------------------------------------

class Filename {

	public:

		Filename();
		Filename( const std::string & fname );

		const std::string & Str() const;

		std::string Base() const;
		std::string BaseNoExt() const;
		std::string Dir() const;
		std::string Drive() const;
		std::string Ext() const;

	private:

		std::string mName;

};

//---------------------------------------------------------------------------
// Read whole file into string/buffer
//---------------------------------------------------------------------------

unsigned int FileRead( std::istream & is, std::vector <char> & buff );
void FileRead( std::ifstream & ifs, std::string & s );
void FileRead( const std::string & fname, std::string & s  );

//---------------------------------------------------------------------------
// Check file/dir esists.
//---------------------------------------------------------------------------

bool FileExists( const std::string & fname );
bool DirExists( const std::string & dirname );

//------------------------------------------------------------------------

}	// end namespace

#endif



