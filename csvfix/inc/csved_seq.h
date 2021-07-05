//---------------------------------------------------------------------------
// csved_seq.h
//
// add sequence numbers to csv
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_SEQ_H
#define INC_CSVED_SEQ_H

#include "a_base.h"
#include "csved_command.h"

namespace CSVED {

//---------------------------------------------------------------------------

class SeqCommand : public Command {

	public:

		SeqCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		void AddSeq( CSVRow & row );
		void ProcessFlags( const ALib::CommandLine & cmd );
		std::string MaskSeq( const std::string & sn ) const;

		int mSeqNo;
		unsigned int mPad;
		unsigned int mCol;
		int mInc;
		std::string mMask;

};

//------------------------------------------------------------------------

}	// end namespace

#endif

