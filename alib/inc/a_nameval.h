//---------------------------------------------------------------------------
// a_nameval.h
//
//
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_NAMEVAL_H
#define INC_A_NAMEVAL_H

#include "a_base.h"
#include "a_dict.h"

namespace ALib {

//---------------------------------------------------------------------------
// Interface for name/value pairs
//---------------------------------------------------------------------------

class NVPSource {

	public:

		NVPSource();
		virtual ~NVPSource();

		virtual void Clear() = 0;
		virtual bool Contains( const std::string & name ) const = 0;
		virtual std::string Value( const std::string & name ) const = 0;
		virtual std::string Value( const std::string & name,
							const std::string & defval ) const = 0;

		virtual unsigned int Names( std::vector <std::string> & names ) const = 0;

	protected:

		bool Parse( const std::string & line,
					  std::string & name,
					  std::string & val ) const;
};

//---------------------------------------------------------------------------
// Implemented on dictionary
//---------------------------------------------------------------------------

class NVPDictSource : public NVPSource {

	public:

		NVPDictSource();
		~NVPDictSource();

		virtual void Clear();
		virtual bool Contains( const std::string & name ) const;
		virtual std::string Value( const std::string & name ) const;
		virtual std::string Value( const std::string & name,
							const std::string & defval ) const;

		virtual unsigned int Names( std::vector <std::string> & names ) const;

		virtual void Add( const std::string & name, const std::string & value );
		virtual void Add( const std::string & line );

	private:

		ALib::Dictionary <std::string> mDict;

};


//------------------------------------------------------------------------
// Dictioanary loaded from text file
//---------------------------------------------------------------------------

class NVPFileSource : public NVPDictSource {

	public:

		NVPFileSource();
		NVPFileSource( const std::string & filename );

		void LoadFrom( const std::string & filename );

};

//---------------------------------------------------------------------------
// Dictionary loaded from string
//---------------------------------------------------------------------------

class NVPStringSource : public NVPDictSource {

	public:

		NVPStringSource();
		NVPStringSource( const std::string & s , char sep = '|' );

		void LoadFrom( const std::string & s, char sep = '|' );

};



//------------------------------------------------------------------------

}	// namespace

#endif

