//---------------------------------------------------------------------------
// a_env.h
//
// environment & command line stuff for alib
//
// Copyright (C) 2008 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_A_ENV_H
#define INC_A_ENV_H

#include <string>
#include <vector>
#include "a_base.h"
#include "a_dict.h"

namespace ALib {


//---------------------------------------------------------------------------
/// The CommandLineFlag class is used in conjunction with the CommandLine
/// class to specify allowed command line flags for an application.
///
/// For example:
/// <pre>    CommandLineFlag f( "-input", true, 1, false ); </pre>
/// creates a flag whose name is "-input", must appear once on the command
/// line and takes a single parameter. The flag must be added to a
/// CommandLine instance using the CommandLine::AddFlag function. Once that
/// has been done the flag can be used by an executable like this:
/// <pre>    mycmd -input foo.txt </pre>
/// If the conditions specified by the flag are not met at run-time, an
/// exception of type ALib::Exception is thrown.
//---------------------------------------------------------------------------

class CommandLineFlag {

	public:

		/// Create command line flag

		/// @param name name of flag as it appears in use on command line
		/// @param reqd indicates if the flag is a required flag
		/// @param pcount specifies number of flag parameters (must be 1 or 0)
		/// @param multi can flag appear on command line more than once?
		CommandLineFlag( const std::string & name,
							bool reqd = false,
							unsigned int pcount = 0,
							bool multi = false );

		///Flag name, including any leading dashes, slashes etc.
		std::string Name() const;

		/// Is this flag required?
		bool Required() const;

		/// How many parameters can this flag take?
		/// Currently only 0 or 1 parameters are supported.
		unsigned int ParamCount() const;

		/// Can this flag appear multiple times on the command line?
		bool MultipleOK() const;

	private:

		std::string mName;
		bool mRequired;
		unsigned int mParamCount;
		bool mMulti;
};


//---------------------------------------------------------------------------
/// The CommandLine class encapsulates the argc and argv parameters of main()
/// and provides means of  checking that the parameters are valid.
///
/// This a <b>singleton</b> and should be initialised  in main(). For example:
/// <pre>
/// int main( int argc, char * argv[] ) {
///    CommandLine cl( argc, argv );
///    ...
/// }
/// </pre>
/// Once constructed, flags can be added to the command line which can be
/// checked and parsed later:
/// <pre>
/// Commandline cl( argc, argv );
/// cl.AddFlag( CommandLineFlag( "-input", truw, 1 ) );
/// ...
/// cl.CheckFlags();
/// </pre>
/// To obtain the value of a parameter, use the GetValue member:
/// <pre>
/// string inval = cl.GetValue( "-input" );
/// </pre>
//---------------------------------------------------------------------------

class CommandLine {

	public:

		/// Construct singleton from main() params.Set instance pointer.
		CommandLine( int arrgc, char * argv[] );

		/// Reset instance pointer.
		~CommandLine();

        /// Get singleton instance. Raises exception if there isn't one.
		static  CommandLine * Instance();

		/// Get number of command line arguments - same as arg in main()
		int Argc() const;

		/// Get command line argument. same as argv[] in main(), but
		/// performs bounds checking.
		const std::string & Argv( int i ) const;

		/// How many flags on command line?
		unsigned int FlagCount( const std::string & name ) const;

		/// Find flag by name. names are case sensitive and include
		/// leading hhyphens, slashes etc.
		int FindFlag( const std::string & name ) const;

		/// Is there at least one instance of flag on comamnd line?
		bool HasFlag( const std::string & name ) const;

		/// Get value of flag or default if flag not on command line
		std::string GetValue( const std::string & name,
								const std::string & defval = "" ) const;

		/// Get list of values for named flag
		unsigned int GetValues( const std::string & name,
								std::vector <std::string> & vals,
								const std::string & defval = "" ) const;

		/// Get flag value or throw exception if not on command line.
		std::string MustGetValue( const std::string & name ) const;

		/// add command line flag to command
		void AddFlag( const CommandLineFlag & f );

		/// Perform checking of all added CommandLineFlags against
		/// the actual command line, throwing exception to report errors.
		/// 'start' is  position to start checking at. The default of 1 is
		/// the first position after the executable name.
		void CheckFlags( unsigned int start = 1 );

		/// Build alist of files. Files must come after flags, unless a
		/// flag itself takes a filename as a parameter
		/// 'start' is  index in command line to start filename list at
		unsigned int BuildFileList( unsigned int start );

		/// How many files on command line?
		unsigned int FileCount() const;

		/// Get file using file list (not command line) index
		std::string File( unsigned int i ) const;

		void Clear();
		void Add( const std::string & arg );

	private:

		bool HasFlag( const std::string & name, unsigned int start );
		void CheckNoMoreFlags( unsigned int start );
		void CheckRequiredFlags( unsigned int start );
		void CheckMultiFlags( unsigned int start );

		static CommandLine * mInstance;

		std::vector <std::string> mArgs;
		std::vector <std::string> mFiles;
		Dictionary <CommandLineFlag> mFlagDict;
};

//---------------------------------------------------------------------------
// Environment wraps the non-standard char *env[] parameter of main()
//---------------------------------------------------------------------------

class Environment {

	public:

		Environment( char * env[] );

		std::string Get( const std::string & name ) const;
		bool Contains( const std::string & name ) const;

		void DumpOn( std::ostream & os ) const;

	private:

		Dictionary <std::string> mDict;
};

//----------------------------------------------------------------------------
// Get environment without objects
//----------------------------------------------------------------------------

std::string GetEnv( const std::string & ev );
bool EnvContains( const std::string & ev );


}	// end namespace

#endif

