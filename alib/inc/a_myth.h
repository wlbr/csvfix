//---------------------------------------------------------------------------
// a_myth.h
//
// unit testing for alib
// this is re-write of a_test.h
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_MYTH_H
#define INC_A_MYTH_H

#include "a_base.h"
#include "a_str.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace ALib {

//----------------------------------------------------------------------------
// Name of environment var that indicates we are testing
//----------------------------------------------------------------------------

const char * const MYTH_TESTS = "MYTH_TESTS";

//---------------------------------------------------------------------------
// Helper to convert streamble types to std::string.
//---------------------------------------------------------------------------

template <class TYPE> std::string ASTR( const TYPE & t ) {
	std::ostringstream os;
	os << t;
	return os.str();
}

//---------------------------------------------------------------------------
// Test functions must look like this
//---------------------------------------------------------------------------

typedef void(* TestFunction)( class TestSuite * );

//---------------------------------------------------------------------------
// Each test is a single function call, wrapped in try-block
//---------------------------------------------------------------------------

class Test {

	public:

		Test( const std::string & name, TestFunction func );

		const std::string & Name() const;
		const std::string & LastException() const;

		void Execute( class TestSuite * s );

	private:

		std::string mName;
		TestFunction mFunc;
		std::string mExMsg;

};

inline void AddTestSuiteToTestManager( TestSuite * s );

//---------------------------------------------------------------------------
// Suite is a collection of Tests which it runs and then reports on
//---------------------------------------------------------------------------

class TestSuite {

	public:

		TestSuite( const std::string & name,
				const std::string & filename = "",
				bool info = true  );

		const std::string & Name() const;
		const std::string & FileName() const;

		void AddTest( const std::string & name, TestFunction func );
		void RunTests();

		void OutputTestName( std::ostream & os );
		void Out( const std::string & type,
				  const std::string & msg );

		bool ShowInfo() const;
		void SetInfo( bool info );
		void Info( const std::string & msg );
		void Info( int n );
		void Fail( const std::string & msg );
		void FailIf( bool cond, const std::string & msg );
		int Fails() const ;

	private:

		std::string mName, mFile;
		std::vector <Test> mTests;
		Test * mCurrentTest;
		bool mInfo;
		unsigned int mFails, mTestFails;

		void TestSummary();
		void SuiteSummary();
};

//---------------------------------------------------------------------------
// Monostate test manager manages test suites.
//---------------------------------------------------------------------------

class TestManager {

	public:

		static TestManager & Mgr();
		static void AddSuite( TestSuite * s );
		static TestSuite * FindSuite( const std::string & name );
		static int Run( const std::string & name );
		static int RunAll();

	private:

		TestManager() {}
		std::vector <TestSuite *> mSuites;
};

//----------------------------------------------------------------------------
// Test output is used to redirect ostream output so it can be tested.
//----------------------------------------------------------------------------

class TestStream {

	public:

		operator std::ostream & ();
		unsigned int Size();
		std::string At( unsigned int i );
		void Clear();

	private:

		void GetLines();
		std::ostringstream mStream;
		std::vector <std::string> mLines;
};

//---------------------------------------------------------------------------

inline void AddTestSuiteToTestManager( TestSuite * s ) {
	TestManager::AddSuite( s );
}

//---------------------------------------------------------------------------
// Tests should normally perform any output via TestSuite so that
// it can be formatted correctly. Tests do not _have_ to produce output.
//---------------------------------------------------------------------------

template <class TYPE> std::ostream & operator << ( TestSuite & s,
														const TYPE & t ) {
	static std::ostringstream ss;
	if ( s.ShowInfo() ) {
		std::cout << "     ";
		s.OutputTestName( std::cout );
		return std::cout << " <<< " <<  t;
	}
	else {
		ss.str("");
		return ss;
	}
}

//---------------------------------------------------------------------------

struct TestAdder {

	TestAdder( const std::string & name, TestSuite & s, TestFunction f ) {
		s.AddTest( name, f );
	}

};

//----------------------------------------------------------------------------
// Test main - call from main() of prog being tested
//----------------------------------------------------------------------------

int TestMain( int argc, char * argv[] );

//----------------------------------------------------------------------------

}		// end ALib namespace

//---------------------------------------------------------------------------
// Useful macros to wrap adding tests and test pass/fail.
// Note macros don't respect namespace.
// ??? There may be still an occasional bug with parentheses here! ???
//---------------------------------------------------------------------------

// add test to suite
#define ADDTEST( testfunc )												\
	myth_suite_.AddTest( #testfunc, testfunc );							\


// fail if expr is false
#define FAILIF( expr )													\
	if ( (expr)) {														\
		std::string msg_ = ALib::ASTR("Test (")							\
			+ #expr + ALib::ASTR(") failed" ) 							\
			+ " at line " + ALib::ASTR( __LINE__ );						\
		myth_suite_.Fail( msg_ );										\
	}																	\

#define FAILNOT( expr )													\
	if ( ! (expr)) {														\
		std::string msg_ = ALib::ASTR("Test (")							\
			+ #expr + ALib::ASTR(") failed" ) 							\
			+ " at line " + ALib::ASTR( __LINE__ );						\
		myth_suite_.Fail( msg_ );										\
	}																	\


// fail if values fail compare with ==
#define FAILNE( v1, v2 )												\
	if ( !((v1) == (v2)) ) {											\
		std::string msg_ = ALib::ASTR(v1) + " != " + ALib::ASTR(v2)		\
			+ " at line " + ALib::ASTR( __LINE__ );						\
		myth_suite_.Fail( msg_ );										\
	}																	\

#define FAILNEM( v1, v2, msg )											\
	if ( !((v1) == (v2)) ) {											\
		std::string msg_ = ALib::ASTR(v1) + " != " + ALib::ASTR(v2)		\
			+ " at line " + ALib::ASTR( __LINE__ ) + " "  + (msg);		\
		myth_suite_.Fail( msg_ );										\
	}																	\

#define FAILEQ( v1, v2 )												\
	if ( ((v1) == (v2)) ) {												\
		std::string msg_ = ALib::ASTR(v1) + " == " + ALib::ASTR(v2)		\
			+ " at line " + ALib::ASTR( __LINE__ );						\
		myth_suite_.Fail( msg_ );										\
	}																	\

#define STOPEQ( v1, v2 )												\
	if ( ((v1) == (v2)) ) {												\
		std::string msg_ = ALib::ASTR(v1) + " == " + ALib::ASTR(v2)		\
			+ " at line " + ALib::ASTR( __LINE__ );						\
		throw ALib::Exception( "STOPEQ " + msg_ ); 						\
	}																	\

// fail with message if expr is true
#define FAILIFM( expr, amsg )											\
	if ( (expr)) {														\
		std::ostringstream os;											\
		os << amsg;														\
		std::string msg_ = ALib::ASTR("Test (")							\
			+ #expr + ALib::ASTR(") failed" ) 							\
			+ " at line " + ALib::ASTR( __LINE__ )						\
			+ " - " + os.str()											\
			;															\
		myth_suite_.Fail( msg_ );										\
	}																	\


// converse of FAILIF
#define PASSIF( expr )													\
	if ( !(expr)) {														\
		std::string msg_ = ALib::ASTR("Test (")							\
			+ #expr + ALib::ASTR(") failed" ) 							\
			+ " at line " + ALib::ASTR( __LINE__ );						\
		myth_suite_.Fail( msg_ );										\
	}																	\

// converse of FAILIFM
#define PASSIFM( expr, amsg )											\
	if ( !(expr)) {														\
		std::ostringstream os;											\
		os << amsg;														\
		std::string msg_ = ALib::ASTR("Test (")							\
			+ #expr + ALib::ASTR(") failed" ) 							\
			+ " at line " + ALib::ASTR( __LINE__ )						\
			+ " - " + os.str()											\
			;															\
		myth_suite_.Fail( msg_ );										\
	}																	\


// check expression that should throw an exception actually does
#define MUST_THROW( expr )												\
	try {																\
		(expr);															\
		(myth_suite_).Fail( #expr +										\
						std::string( " should throw but didn't" ) );	\
	}																	\
	catch( ... ) {														\
	}																	\

//------------------------------------------------------------------------
// Define a suite - gets name myth_suite_
//------------------------------------------------------------------------

#define DEFSUITE( name )												\
	 static ALib::TestSuite myth_suite_( name, __FILE__ )				\


//---------------------------------------------------------------------------
// Define a test and add it to the suite
//----------------------------------------------------------------------------

#define DEFTEST( testname )												\
		static void testname( TestSuite * s_  = 0 );					\
		static TestAdder TA_##testname( #testname, myth_suite_, testname );  \
		static void testname( TestSuite * s_ )


#define TESTOUT( msg_ )													\
	myth_suite_ << msg_;												\


//----------------------------------------------------------------------------



#endif

