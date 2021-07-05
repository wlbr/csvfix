//---------------------------------------------------------------------------
// csved_sum.h
//
// create summary info from CSV input
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_SUM_H
#define INC_CSVED_SUM_H

#include "a_base.h"
#include "csved_command.h"
#include "csved_types.h"
#include <map>

namespace CSVED {

//---------------------------------------------------------------------------

class SummaryCommand : public Command {

	public:

		SummaryCommand( const std::string & name,
						const std::string & desc );

		int Execute( ALib::CommandLine & cmd );

	private:

		enum Type { Average, Sum, Min, Max, Median, Mode, Frequency, Size };

		void DoMinMax( IOManager & io );
		void DoSum( IOManager & io );
		void DoAvg( IOManager & io );
		void DoFreq( IOManager & io );
		void DoMedian( IOManager & io );
		void DoMode( IOManager & io );

		typedef std::map <int,std::pair<int,int> > SizeMap;
		void RecordSizes( const CSVRow & row, SizeMap & sm );
		void PrintSizes( IOManager & io, const SizeMap & sm );

		void SumCols( std::vector <double> & sums );
		unsigned int  CalcFreqs();
		std::string MakeKey( const CSVRow & row ) const;

		void Summarise( IOManager & io );
		void ProcessFlags( const ALib::CommandLine & cmd );
		void GetFields( const ALib::CommandLine & cmd,
						 const std::string & flsg );

		int Cmp( const CSVRow & r1, const CSVRow & r2 ) const;

		CSVTable mRows;
		Type mType;
		FieldList mFields;

		struct FreqMapEntry {
			FreqMapEntry( int idx )
				: mFreq( 1 ) {
				mIndex.push_back( idx );
			}
			unsigned int mFreq;
			std::vector <unsigned int> mIndex;
		};

		typedef std::map <std::string, FreqMapEntry> FreqMap;
			FreqMap mFreqMap;
};

//------------------------------------------------------------------------

}	// end namespace

#endif

