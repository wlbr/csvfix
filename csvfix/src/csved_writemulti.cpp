//---------------------------------------------------------------------------
// csved_writemulti.cpp
//
// Convert CSV input to multi-line records
//
// Copyright (C) 2014 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_collect.h"
#include "csved_writemulti.h"
#include "csved_cli.h"
#include "csved_strings.h"
#include "csved_fixed.h"

using std::string;

namespace CSVED {

//---------------------------------------------------------------------------
// Register unique command
//---------------------------------------------------------------------------

static RegisterCommand <WriteMultiCommand> rc1_(
	CMD_WRITEMUL,
	"convert CSV to multi-line records"
);


//----------------------------------------------------------------------------
// Help text
//----------------------------------------------------------------------------

const char * const WMHELP_HELP = {
	"convert CSV to multi-line master/detail records\n"
	"usage: csvfix write_multi [flags] [files ...]\n"
	"where flags are:\n"
	"  -m fields\tfields comprising the master record (required)\n"
	"  -d fields\tfields comprising the detail records (default is all not in master)\n"
    "  -rs sep\tspecify record separator (default is none)\n"
	"#SMQ,SEP,IBL,IFN,OFL"
};

//----------------------------------------------------------------------------
// Standard constructor
//----------------------------------------------------------------------------

WriteMultiCommand :: WriteMultiCommand( const string & name,
                                        const string & desc)
    : Command( name, desc, WMHELP_HELP ) {
    AddFlag( ALib::CommandLineFlag( FLAG_MASTER, true, 1 ));
    AddFlag( ALib::CommandLineFlag( FLAG_DETAIL, false, 1 ));
    AddFlag( ALib::CommandLineFlag( FLAG_RECSEP, false, 1 ));
}

//----------------------------------------------------------------------------
// Read input. Whenever master fields change, output a master record. Always
// output a detail record.
//----------------------------------------------------------------------------

int WriteMultiCommand :: Execute( ALib::CommandLine & cmd ) {

    GetSkipOptions( cmd );
    ProcessFlags( cmd );
    IOManager io( cmd );
    CSVRow row, master;
    bool haveout = false;
    while( io.ReadCSV( row ) ) {
		if ( Skip( row ) ) {
			continue;
		}
        if ( GetNewMaster( row, master )) {    // new master
            if( haveout) {
                WriteRecordSeparator( io );
            }
            else {
                haveout = true;
            }
            io.WriteRow( master );
        }
        io.WriteRow( MakeDetail( row ));
    }
    WriteRecordSeparator( io );

    return 0;
}

//----------------------------------------------------------------------------
// Output record separator
//----------------------------------------------------------------------------

void WriteMultiCommand :: WriteRecordSeparator( IOManager & io ) {
    if ( mHaveRecSep ) {
        CSVRow tmp;
        tmp.push_back( mRecSep );
        io.WriteRow( tmp );
    }
}

//----------------------------------------------------------------------------
// Create detail record. If detail fields not specified, output all
// fields which are not in the master field list.
//----------------------------------------------------------------------------

CSVRow WriteMultiCommand :: MakeDetail( const CSVRow & row ) {
    CSVRow detail;
    if ( mDetail.empty() ) {
        for ( unsigned int i = 0; i < row.size(); i++ ) {
            if ( ! ALib::Contains( mMaster, i )) {
                detail.push_back( row[i] );
            }
        }
    }
    else {
        for ( unsigned int i = 0; i < mDetail.size(); i++ ) {
            detail.push_back( GetField( row, mDetail[i] ));
        }
    }
    return detail;
}

//----------------------------------------------------------------------------
// Set master fields from row using field list specified by user.
//----------------------------------------------------------------------------

void WriteMultiCommand :: SetMasterFields( const CSVRow & row, CSVRow & master ) {
    master.clear();
    for ( unsigned int i = 0; i < mMaster.size(); i++ ) {
        master.push_back( GetField( row, mMaster[i]));
    }
}

//----------------------------------------------------------------------------
// See if we have a new master record.
//----------------------------------------------------------------------------

bool WriteMultiCommand :: GetNewMaster(const CSVRow & row, CSVRow & master) {
    if ( master.empty() ) {
        SetMasterFields( row, master );
        return true;
    }
    else {
        CSVRow tmp;
        SetMasterFields( row, tmp );
        if ( tmp == master ) {
            return false;
        }
        else {
            master = tmp;
            return true;
        }
    }
}

//----------------------------------------------------------------------------
// Must have a list of master fields. If no detail feilds are provided we
// will deduce them later.
//----------------------------------------------------------------------------

void WriteMultiCommand :: ProcessFlags( const ALib::CommandLine & cmd ) {

    if ( ! cmd.HasFlag( FLAG_MASTER ) ) {
        CSVTHROW( "Need to specify master fields with "
                    << FLAG_MASTER << " option");
    }

    ALib::CommaList ml( cmd.GetValue( FLAG_MASTER, "" ));
    if ( ml.Size() == 0 ) {
        CSVTHROW( "Must specify at least one master field with "
                    << FLAG_MASTER << " option");
    }
	CommaListToIndex( ml, mMaster );

    ALib::CommaList dl( cmd.GetValue( FLAG_DETAIL, "" ));
	CommaListToIndex( dl, mDetail );

    mHaveRecSep = cmd.HasFlag( FLAG_RECSEP );
    mRecSep = cmd.GetValue( FLAG_RECSEP, "" );

}

//----------------------------------------------------------------------------

} // namespace
