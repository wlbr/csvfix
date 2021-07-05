#include "a_env.h"
#include "a_globswitch.h"
#include <iostream>
using namespace std;

//static ALib::GlobSwitch gs( false );

int _CRT_glob = 1;

int main( int argc, char * argv[] ) {
	ALib::CommandLine cl( argc, argv );
	for ( unsigned int i = 0; i < argc; i++ ) {
		cout << argv[i] << endl;
	}

	cout << "----------------------------------" << endl;

	cl.BuildFileList( 1 );
	for ( unsigned int i = 0; i < cl.FileCount(); i++ ) {
		cout << cl.File( i ) << endl;
	}
}

