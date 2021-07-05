//---------------------------------------------------------------------------
// a_regex.h
//
// Regular expressions for ALib, using algorithm adapted from Software Tools
// In Pascal. Sample use:
//
//		string s = "Ah Sunflower, weary of time";
//		RegEx ex( "[Ss].*," );
//		RegEx::Pos p = ex.FindIn( s );
//
//  At this point:
//
//		p.Found()	- returns true;
//		p.Start()	- returns 3 (index of 'S')
//		p.Length()	- returns 10 (length of "Sunflower,"
//
//
// Copyright (C) 2006 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_REGEX_H
#define INC_A_REGEX_H

#include "a_base.h"

namespace ALib {

//---------------------------------------------------------------------------

class RegEx {

	public:

		class Encoding;
		class CharBitMap;

		class Pos {

			friend class RegEx;

			public:

				Pos();
				Pos( unsigned int start,
							unsigned int len,
							bool found );

				unsigned int Start() const;
				unsigned int Length() const;

				operator bool() const;
				bool Found() const;

			private:

				unsigned int mStart, mLen;
				bool mFound;
		};

		enum CaseSense { Sensitive, Insensitive };

		RegEx( const std::string & expr = "",
							CaseSense csense = Sensitive );
		RegEx( const RegEx & ex );
		void operator =( const RegEx & ex );
		~RegEx();

		Pos FindIn( const std::string & s,
							unsigned int start = 0 ) const;
		void Clear();

		std::string GetEncoding() const;
		Pos ReplaceIn( std::string & s, const std::string & v,
									unsigned int start = 0 );


		unsigned int ReplaceAllIn( std::string & s,
									const std::string & v );

		std::string SavedMatch( unsigned int i ) const;
		unsigned int SavedMatchCount() const;

		static std::string Escape( const std::string & s );

	private:

		Pos MatchAt( const std::string & s, unsigned int start,
						unsigned int eindex ) const;

		Pos MatchEmpty() const;

		bool MatchChar( const std::string & s, unsigned int si,
							unsigned int eindex ) const;
		unsigned int MatchClosure( const std::string & s,
									unsigned int start,
									unsigned int eindex,
									unsigned int matchlen ) const;

		std::string ReplaceSaved( const std::string & s ) const;

		Encoding * mEnc;

};

//------------------------------------------------------------------------

} // namespace

#endif

