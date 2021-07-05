//------------------------------------------------------------------------
// a_myth.cpp
//
// Testing stuff for alib - replaces a_test.
//
// Copyright (C) 2008 Neil Butterworth
//------------------------------------------------------------------------

#include "a_base.h"
#include "a_myth.h"
#include <iostream>

using std::string;

namespace ALib {

//----------------------------------------------------------------------------
// single named test
//----------------------------------------------------------------------------

Test :: Test( const std::string & name, TestFunction func )
	: mName( name ), mFunc( func ) {
}

const std::string & Test :: Name() const {
	return mName;
}

// text of last exception message
const std::string & Test :: LastException() const {
	return mExMsg;
}

// execute a test, wrapped in a try block
void Test :: Execute( class TestSuite * s ) {
	mExMsg = "";
	try {
		mFunc( s );
	}
	catch( const std::exception & e ) {
		mExMsg = "Exception of type std::exception: "
						+ std::string(e.what());
	}
	catch( const std::string & e ) {
		mExMsg = "Exception of type std::string: " + e;
	}
	catch( const char * e ) {
		mExMsg = "Exception of type const char*: " + std::string(e);
	}
	catch( ... ) {
		mExMsg = "Exception of unknown type";
	}
}

//----------------------------------------------------------------------------
// Test suite is a collection of tests
//----------------------------------------------------------------------------

TestSuite :: TestSuite( const std::string & name,
						const std::string & filename ,
						bool info   )
	: mName( name ), mFile( filename ),
		 mCurrentTest(0), mInfo( info ) {
	AddTestSuiteToTestManager( this );
}

const std::string & TestSuite :: Name() const {
	return mName;
}

const std::string & TestSuite :: FileName() const {
	return mFile;
}

void TestSuite :: AddTest( const std::string & name, TestFunction func ) {
	mTests.push_back( Test( name, func ) );
}

void TestSuite :: RunTests() {
	mCurrentTest = 0;
	mTestFails = 0;
	std::cout << "     [" << mName ;
	if ( mFile != "" ) {
		std::cout << " - "  << mFile;
	}
	std::cout << "]" << std::endl;
	for ( unsigned int i = 0; i < mTests.size(); i++ ) {
		mFails = 0;
		mCurrentTest = & mTests[i];
		Out( "    ", "" );
		mCurrentTest->Execute( this );
		TestSummary();
	}
	mCurrentTest = 0;
	SuiteSummary();
}


void TestSuite :: OutputTestName( std::ostream & os ) {
	std::string tn = "[" + Name();
	if ( mCurrentTest != 0 ) {
		tn += " - " + mCurrentTest->Name();
	}
	tn += "] ";
	os << tn;
}

void TestSuite :: Out( const std::string & type,
		  const std::string & msg ) {
	std::cout << type << ' ';
	OutputTestName( std::cout );
	std::cout << ' ' << msg << std::endl;
}

bool TestSuite :: ShowInfo() const {
	return mInfo;
}

void TestSuite :: SetInfo( bool info ) {
	mInfo = info;
}

void TestSuite :: Info( const std::string & msg ) {
	if ( mInfo ) {
		Out( "INFO", msg );
	}
}

void TestSuite :: Info( int n ) {
	Info( ALib::ASTR(n) );
}

void TestSuite :: Fail( const std::string & msg ) {
	Out( "FAIL", msg );
	mFails++;
}

void TestSuite :: FailIf( bool cond, const std::string & msg ) {
	if ( cond ) {
		Fail( msg );
	}
}

int TestSuite :: Fails() const {
	return mTestFails;
}

void TestSuite :: TestSummary() {
	if ( mCurrentTest->LastException() != "" ) {
		Out( "XCPT", mCurrentTest->LastException() );
		mFails++;
	}
	if ( mFails > 0 ) {
		mTestFails++;
		Out( "FAIL", "Test had " + ALib::ASTR(mFails) + " failures" );
	}
}

void TestSuite :: SuiteSummary() {
	if ( mTestFails > 0 ) {
	//	Out( "\nFAIL", ALib::ASTR(mTestFails) + " tests out of "
	//			+ ALib::ASTR( mTests.size()) + " failed" );
	}
	else {
	//	Out( "\nOK  ", "All tests passed\n" );
	}
}

//----------------------------------------------------------------------------
// Tes manager methods are all static
//----------------------------------------------------------------------------


TestManager & TestManager :: Mgr() {
	static TestManager mgr;
	return mgr;
}

void TestManager :: AddSuite( TestSuite * s ) {
	if ( Mgr().FindSuite( s->Name() ) ) {
		std::cerr << "Duplicate test suite: " << s->Name() << std::endl;
	}
	else {
		Mgr().mSuites.push_back( s );
	}
}

TestSuite * TestManager :: FindSuite( const std::string & name ){
	for ( unsigned int i = 0; i < Mgr().mSuites.size(); i++ ) {
		if ( Mgr().mSuites[i]->Name() == name ) {
			return Mgr().mSuites[i];
		}
	}
	return 0;
}

int TestManager :: Run( const std::string & name ) {
	TestSuite * s = FindSuite( name );
	if ( s == 0 ) {
		std::cerr << "==== No such test suite: " << name << " ====" << std::endl;
		return -1;
	}
	else {
		s->RunTests();
		return s->Fails();
	}
}

int TestManager :: RunAll() {
	int n = 0;
	for ( unsigned int i = 0; i < Mgr().mSuites.size(); i++ ) {
		Mgr().mSuites[i]->RunTests();
		n += Mgr().mSuites[i]->Fails();
	}
	return n;
}

//----------------------------------------------------------------------------
// Stram for output testing
//----------------------------------------------------------------------------

TestStream :: operator std::ostream & () {
// TODO (neilb#1#): Clearing test stream not working
	return mStream;
}

void TestStream :: Clear() {
	mStream.str("");
}

unsigned int TestStream :: Size() {
	GetLines();
	return mLines.size();
}

std::string TestStream :: At( unsigned int i ) {
	GetLines();
	return mLines.at(i);
}

void TestStream :: GetLines() {
	std::string s = mStream.str();
	if ( s != "" ) {
		mStream.str("");
		mStream.clear();
		std::vector <std::string> v;
		ALib::Split( s, '\n', v );
		for ( unsigned int i = 0; i < v.size(); i++ ) {
			std::string s = ALib::Trim( v[i] );
			if ( s != "" ) {
				mLines.push_back( ALib::Trim( v[i] ) );
			}
		}
	}
}

//----------------------------------------------------------------------------
// Main function for test
// ??? currently just testing some ideas ???
//----------------------------------------------------------------------------

int TestMain( int argc, char *argv[] ) {

	if ( argc == 1 ) {
		int n =  TestManager::RunAll();
		if ( n != 0 ) {
			std::cout << "\n####### " << n << " tests failed #######\n";
		}
		else {
			std::cout << "\n-----  ALL TESTS PASSED -----"  << std::endl;
		}
		return 	n;
	}
	else if ( string( argv[1] ) == "-ask" ) {
		string name;
		while( 1 ) {
			std::cout << "Test suite: ";
			std::getline( std::cin, name );
			if ( TestManager::FindSuite( name ) ) {
				int n = TestManager::Run( name );
				return n;
			}
			else {
				std::cout << "No such test suite" << std::endl;
			}
		}

	}
	else {
		int n = 0;
		for ( int i = 1; i < argc; i++ ) {
			n += TestManager::Run( argv[i] );
		}
		if ( n != 0 ) {
			std::cout << "\n####### " << n << " tests failed #######\n";
		}
		return 	n;
	}
	return -1;
}

//----------------------------------------------------------------------------

} // namespace


// end
