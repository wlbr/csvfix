//----------------------------------------------------------------------------
// csved_sparse.h
//
// Simple CSV parser class for CSV validation only.
//
// Copyright (C) 2011 Neil Butterworth
//----------------------------------------------------------------------------

#ifndef INC_CSVED_SPARSE_H
#define INC_CSVED_SPARSE_H

#include <vector>
#include <string>
#include <iostream>
#include "csved_types.h"

namespace CSVED {

//----------------------------------------------------------------------------

class CSVChecker {

	public:

		CSVChecker( const std::string & fname,
					std::istream & src,
					char fieldsep = ',',
					bool dqspecial = true,
					bool embednlok = true  );

		bool NextRecord( CSVRow & r );

	private:

		bool AtEndRec();
		bool AtEndField() const;
		bool IsFieldSep() const;
		bool HaveQuote() const;

		void ReadField( CSVRow & r );
		void ReadQuotedField( CSVRow & r );

		void NextChar();
		char Peek();

		void Error( const std::string & msg, bool context  );

		std::string mFileName;
		std::istream & mSrc;
		char mNext, mFieldSep;
		bool mDQSpecial, mEmbedNLOK;
		unsigned int mLineNo;
		std::string mLine;
};

} // namespace

#endif

// end

