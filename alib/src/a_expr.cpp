//----------------------------------------------------------------------------
// a_expr.cpp
//
// simple expression eveluation
//
// Copyright (C) 2009 Neil Butterworth
//----------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_expr.h"
#include "a_collect.h"
#include "a_rand.h"
#include "a_date.h"
#include "a_time.h"
#include "a_regex.h"

#include <stack>
#include <iostream>
#include <cmath>
#include <cstring>
#include <ctime>
#include <sstream>
#include <iomanip>

using std::string;
using std::vector;
using std::deque;

//----------------------------------------------------------------------------

namespace ALib {

//----------------------------------------------------------------------------
// Hack for user seeding of rng
//----------------------------------------------------------------------------

bool Expression :: mUseRNGSeed = false;
int Expression :: mRNGSeed = 0;

void Expression :: SetRNGSeed( int n ) {
	mUseRNGSeed = true;
	mRNGSeed = n;
}

int Expression :: GetRNGSeed() {
	if ( mUseRNGSeed ) {
		return mRNGSeed;
	}
	else {
		return std::time(0);
	}
}

//----------------------------------------------------------------------------
// Separator used between expressions
//----------------------------------------------------------------------------

const char * EXPR_SEP = ";";

//----------------------------------------------------------------------------
// Names of operators that cannot be used directly by user
//----------------------------------------------------------------------------

const char * UMINUS_STR 	= "UM";		// unary minus
const char * RDVAR_STR 	= "RV";		// read variable op
const char * FNCALL_STR 	= "FC";		// call function op

//----------------------------------------------------------------------------
// Characters used to denote special tokens
//----------------------------------------------------------------------------

const char CHAR_DQUOTE		= '"';		// strings are delimeted by these
const char CHAR_SQUOTE		= '\'';		// or these
const char CHAR_VAR		= '$';		// variables are preceded by these
const char CHAR_ESC		= '\\';		// escappe next char

//----------------------------------------------------------------------------
// The tokeniser takes a string containing an expression and chops it up
// into tokens. Used by the expression compiler.
//----------------------------------------------------------------------------

class ExprTokeniser {

	public:

		ExprTokeniser( const std::string & expr );
		ExprToken Get();

		void DumpOn( std::ostream & os ) const;

	private:


		bool ReadWS();
		char Next();
		char Peek() const;

		ExprToken ReadNum();
		ExprToken ReadStr();
		ExprToken ReadVar();
		ExprToken ReadOp();
		ExprToken ReadFunc();

		ExprToken Error( const string & msg ) const;

		bool mCanBeUnaryMinus;
		std::string mExprs;
		char mCurrent;
		unsigned int mPos, mCol, mLine;

};

//----------------------------------------------------------------------------
// Compiler taks a string containing one or more expressions and produces
// an intermediate reverse poliish form that can be handled easily by
// the ectual expression evaluator
// TODO (neilb#1#): maybe change to use exceptions rather than string returns
//----------------------------------------------------------------------------

class ExprCompiler {

	public:

		ExprCompiler();
		~ExprCompiler();

		std::string Compile( const std::string & expr,
								Expression::RPNRep & rep );

		static void DumpRP( std::ostream & os, const Expression::RPNRep & rp );

	private:

		void PopSubExpr( Expression::RPNRep & rep );
		void PopHigherPrec( const ExprToken & tok, Expression::RPNRep & rep );
		std::stack <ExprToken> mStack;
		std::stack <std::string> mFuncStack;
};

//----------------------------------------------------------------------------
// Singleton dictionary used to map funcrtion names to actual functions
//----------------------------------------------------------------------------

Dictionary <Expression::AddFunc> Expression::mFuncs;

//----------------------------------------------------------------------------
// helper macrro to add function to dictionary
//----------------------------------------------------------------------------

#define ADD_FUNC( name, fn, np ) 							\
	static Expression::AddFunc reg_##fn##_( name, fn, np )

//----------------------------------------------------------------------------
// static method that does the real addi to dictionary
//----------------------------------------------------------------------------

void Expression :: AddFunction( const string & name, const AddFunc & f ) {
	mFuncs.Add( name, f );
}

//----------------------------------------------------------------------------
// helper to get double version of  function param - params are strings
//----------------------------------------------------------------------------

static double GetDParam( const deque <string> & params, int i ) {
	string s = params.at( i );
	if ( ! IsNumber( s ) ) {
		ATHROW( "Invalid number: " << s );
	}
	return ToReal( s );
}

//----------------------------------------------------------------------------
// The following are the functions we currently support
//----------------------------------------------------------------------------

// if first param is true, return second param else return third
static string FuncIf( const deque <string> & params, Expression * ) {
	if ( Expression::ToBool( params[0] ) ) {
		return params[1];
	}
	else {
		return params[2];
	}
}

// Invert truth of param . We don't currently support '!' operator.
static string FuncNot( const deque <string> & params, Expression *  ) {
	if ( Expression::ToBool( params[0] ) ) {
		return "0";
	}
	else {
		return "1";
	}
}

// Transform double param into an integer
static string FuncInt( const deque <string> & params, Expression *  ) {
	double n = GetDParam( params, 0 );
	return Str( (int)n );
}

// Get absolute value of param
static string FuncAbs( const deque <string> & params, Expression *  ) {
	double n = GetDParam( params, 0 );
	return Str( std::fabs( n ) );
}

// Get sign of param
static string FuncSign( const deque <string> & params, Expression *  ) {
	double n = GetDParam( params, 0 );
	if ( n == 0 ) {
		return "0";
	}
	else if ( n > 0 ) {
		return "1";
	}
	else {
		return "-1";
	}
}

// Trim leading & trailing whitespace from param
static string FuncTrim( const deque <string> & params, Expression *  ) {
	return Str( Trim( params[0] ) );
}

// Return string converted to uppercase
static string FuncUpper( const deque <string> & params, Expression *  ) {
	return Str( Upper( params[0] ) );
}

// Return string converted to lowercase
static string FuncLower( const deque <string> & params, Expression *  ) {
	return Str( Lower( params[0] ) );
}

// Return length of param treaded as string
static string FuncLen( const deque <string> & params, Expression *  ) {
	return Str( params[0].size() );
}

// Return substring of first param specified by start and length
static string FuncSubstr( const deque <string> & params, Expression *  ) {
	int pos = int( GetDParam( params, 1 ) ) - 1;
	if ( pos < 0 ) {
		ATHROW( "Invalid position in substr()" );
	}
	int len = int( GetDParam( params, 2 ) );
	if ( len < 0 ) {
		ATHROW( "Invalid length in substr()" );
	}

	return params[0].substr( pos, len );
}

// Get position of second param in first. Returns zero on fail else
// one-based index of start of second param in first.
static string FuncPos( const deque <string> & params, Expression *  ) {
	string haystack = params[0];
	string needle = params[1];
	STRPOS pos = haystack.find( needle );
	return Str( pos == STRNPOS ? 0 : pos + 1 );
}

// Is param a number (real or integer)
static string FuncIsNum( const deque <string> & params, Expression *  ) {
	return IsNumber( params[0] ) ? "1" : "0";
}

// Normalise param into boolean 1 (true) or 0 (false)
static string FuncBool( const deque <string> & params, Expression *  ) {
	if ( Expression::ToBool( params[0] ) ) {
		return "1";
	}
	else {
		return "0";
	}
}

// Does param consist of only whitespace characters
static string FuncIsEmpty( const deque <string> & params, Expression *  ) {
	return params[0].find_first_not_of( " \t"  ) == STRNPOS ? "1" : "0";
}

// return random number
static string FuncRandom( const deque <string> & params, Expression *  ) {
	static RandGen rg( Expression::GetRNGSeed() );
	return Str( rg.NextReal() );
}

// get current date in ISO format
static string FuncToday( const deque <string> & params, Expression *  ) {
	Date d = Date::Today();
	return d.Str();
}

// get current time in hh:mm:ss format
static string FuncNow( const deque <string> & params, Expression *  ) {
	Time t = Time::Now();
	return t.Str();
}

// compare params as strings ignoring case
static string FuncStrEq( const deque <string> & params, Expression *  ) {
	return Str( Equal( params[0], params[1]) );
}

// see if regex matches string
static string FuncMatch( const deque <string> & params, Expression *  ) {
	RegEx re( params[1] );
	RegEx::Pos pos = re.FindIn( params[0] );
	return pos.Found() ? "1" : "0";
}

// get environment variable, or empty string
static string FuncGetenv( const deque <string> & params, Expression *  ) {
	const char * val = std::getenv( params[0].c_str() );
	return val == NULL ? "" : val;
}

// min and max
static string FuncMin( const deque <string> & params, Expression *  ) {
	if ( IsNumber( params[0] ) && IsNumber( params[1] )) {
		double n1 = GetDParam( params, 0 );
		double n2 = GetDParam( params, 1 );
		return n1 < n2 ? params[0] : params[1];
	}
	else {
		return params[0] < params[1] ? params[0] : params[1];
	}
}

static string FuncMax( const deque <string> & params, Expression *  ) {
	if ( IsNumber( params[0] ) && IsNumber( params[1] ))  {
		double n1 = GetDParam( params, 0 );
		double n2 = GetDParam( params, 1 );
		return n1 > n2 ? params[0] : params[1];
	}
	else {
		return params[0] > params[1] ? params[0] : params[1];
	}
}

// date validation and element access
static string FuncIsDate( const deque <string> & params, Expression *  ) {
	try {
		Date d( params[0] );
	}
	catch( ... ) {
		return "0";
	}
	return "1";
}

static string FuncDay( const deque <string> & params, Expression *  ) {
	try {
		Date d( params[0] );
		return Str( d.Day() );
	}
	catch( ... ) {
		return "";
	}
}

static string FuncMonth( const deque <string> & params, Expression *  ) {
	try {
		Date d( params[0] );
		return Str( d.Month() );
	}
	catch( ... ) {
		return "";
	}
}

static string FuncYear( const deque <string> & params, Expression *  ) {
	try {
		Date d( params[0] );
		return Str( d.Year() );
	}
	catch( ... ) {
		return "";
	}
}

// get 1-based index of first param in comma-list
static string FuncIndex( const deque <string> & params, Expression *  ) {
	CommaList cl( params[1] );
	int idx = cl.Index( params[0] );
	return Str( idx + 1 );
}

// pick 1-based value from comma list
static string FuncPick( const deque <string> & params, Expression *  ) {
	if ( ! IsInteger( params[0] )) {
		ATHROW( "First parameter of pick() must be integer" );
	}
	int n = ToInteger( params[0] );
	CommaList cl( params[1] );
	if ( n < 1 || n > (int) cl.Size() ) {
		return "";
	}
	else {
		return cl.At( n - 1 );
	}
}

// get field from current record - index is 1-based
static string FuncField( const deque <string> & params, Expression * e ) {
	if ( ! IsInteger( params[0] )) {
		ATHROW( "Parameter of field() must be integer" );
	}
	int i =  ToInteger( params[0] ) - 1;
	if ( i < 0 || i >= (int) e->PosParamCount() ) {
		return "";
	}
	else {
		return e->PosParam( i );
	}
}

// check if number is an integer
static string FuncIsInt( const deque <string> & params, Expression * e ) {
	return IsInteger( params[0] ) ? "1" : "0";
}

// try to match regex agains all  positional parameters
// returns 1-based index of matching parameter, or 0 on no match
static string FuncFind( const deque <string> & params, Expression * e ) {
	RegEx re( params[0] );
	for ( unsigned int i = 0; i < e->PosParamCount(); i++ ) {
		RegEx::Pos pos = re.FindIn( e->PosParam( i ) );
		if ( pos.Found() ) {
			return Str( i + 1 );
		}
	}
	return "0";
}

// round number n to d decimal places
static string FuncRound( const deque <string> & params, Expression * e ) {
	if ( ! IsInteger( params[1] )) {
		ATHROW( "Second parameter of round() must be integer" );
	}
	double n = IsNumber( params[0] ) ? ToReal( params[0] ) : 0.0 ;
	int d = ToInteger( params[1] );
	if ( d < 0 ) {
		ATHROW( "Second parameter of round() must be non-negative" );
	}
	std::ostringstream os;
	os << std::fixed << std::setprecision( d ) << n;
	return os.str();
}

//----------------------------------------------------------------------------
// Add all the functions to the function dictionary
//----------------------------------------------------------------------------

ADD_FUNC( "abs", 		FuncAbs, 		1 );
ADD_FUNC( "bool", 		FuncBool, 		1 );
ADD_FUNC( "day", 		FuncDay, 		1 );
ADD_FUNC( "env", 		FuncGetenv, 	1 );
ADD_FUNC( "field", 		FuncField, 		1 );
ADD_FUNC( "find", 		FuncFind, 		1 );
ADD_FUNC( "if", 		FuncIf, 		3 );
ADD_FUNC( "index", 		FuncIndex, 		2 );
ADD_FUNC( "int", 		FuncInt, 		1 );
ADD_FUNC( "isdate", 	FuncIsDate, 	1 );
ADD_FUNC( "isempty", 	FuncIsEmpty,	1 );
ADD_FUNC( "isint", 		FuncIsInt, 		1 );
ADD_FUNC( "isnum", 		FuncIsNum, 		1 );
ADD_FUNC( "month", 		FuncMonth, 		1 );
ADD_FUNC( "not", 		FuncNot, 		1 );
ADD_FUNC( "pos",		FuncPos, 		2 );
ADD_FUNC( "random", 	FuncRandom, 	0 );
ADD_FUNC( "sign",		FuncSign, 		1 );
ADD_FUNC( "substr", 	FuncSubstr, 	3 );
ADD_FUNC( "trim", 		FuncTrim, 		1 );
ADD_FUNC( "today", 		FuncToday, 		0 );
ADD_FUNC( "now", 		FuncNow, 		0 );
ADD_FUNC( "upper", 		FuncUpper, 		1 );
ADD_FUNC( "lower", 		FuncLower, 		1 );
ADD_FUNC( "len", 		FuncLen, 		1 );
ADD_FUNC( "streq", 		FuncStrEq, 		2 );
ADD_FUNC( "match", 		FuncMatch, 		2 );
ADD_FUNC( "max", 		FuncMax, 		2 );
ADD_FUNC( "min", 		FuncMin, 		2 );
ADD_FUNC( "pick", 		FuncPick, 		2 );
ADD_FUNC( "year", 		FuncYear, 		1 );
ADD_FUNC( "round", 		FuncRound, 		2 );


//----------------------------------------------------------------------------
// Operator names and associated precedence
//----------------------------------------------------------------------------

struct OpEntry {
	const char * mOp;		// name
	unsigned int mPrec;	// precedence - high number == high precedence
};

OpEntry Ops[] = {
	{"*", 10}, {"/",10}, {"%",10},
	{"+",8}, {"-",8},
	{"==", 6}, {"<>", 6}, {"!=", 6}, {"<=", 6}, {">=",6}, {"<", 6}, {">",6},
	{".", 4 },
	{"&&", 3 }, { "||", 3 },
	{",", 2 },
	{FNCALL_STR, 1 }, {"(",2}, {")",2},        	// must be low
 	{EXPR_SEP, 0 },								// must be lowest prec
	{NULL, 0}									// must be null terminated
};

//----------------------------------------------------------------------------
// Dump token in readable form. Only used for debug.
//----------------------------------------------------------------------------

void ExprToken :: DumpOn( std::ostream & os ) const {
	switch( mType ) {
		case etNone:		os << "NONE"; break;
		case etOp:			os << "OP  "; break;
		case etNum:			os << "NUM "; break;
		case etStr:			os << "STR "; break;
		case etVar:			os << "VAR "; break;
		case etFunc:		os << "FUNC"; break;
		case etError:		os << "ERR "; break;
		case etDone:		os << "DONE" ; break;
		default:			ATHROW( "Bad token type " << mType );
	}
	os << " [" << mValue << "]" ;
}

//----------------------------------------------------------------------------
// is operator expression separator?
//----------------------------------------------------------------------------

bool ExprToken :: IsSep() const {
	return Type() == etOp && Value() == EXPR_SEP;
}


//----------------------------------------------------------------------------
// create temp separator object for comparisons
// TODO (neilb#1#): should probably be function
//----------------------------------------------------------------------------

ExprToken ExprToken :: MakeSep() {
	return ExprToken( etOp, EXPR_SEP, 0 );
}

//----------------------------------------------------------------------------
// Tokens are equal if have same type and same string rep
//----------------------------------------------------------------------------

bool ExprToken :: operator == ( const ExprToken & et ) const {
	return mType == et.mType && mValue == et.mValue;
}

//----------------------------------------------------------------------------
// Create tokeniser from string to tokenise
//----------------------------------------------------------------------------

ExprTokeniser :: ExprTokeniser( const std::string & e )
	: mCanBeUnaryMinus( true ), mExprs( e ), mPos( 0 )  {
	Next();
}

//----------------------------------------------------------------------------
// Helper to create error token
//----------------------------------------------------------------------------

ExprToken ExprTokeniser :: Error( const string & msg ) const {
	return ExprToken( ExprToken::etError, msg );
}

//----------------------------------------------------------------------------
// get next token, returning special etDone token when  no more inut
//----------------------------------------------------------------------------

ExprToken ExprTokeniser :: Get() {

	while( mCurrent ) {
		if ( isspace( mCurrent ) ) {
			ReadWS();
			continue;
		}
		else if ( isdigit( mCurrent )
					|| (mCurrent == '.' && isdigit( Peek()))) {
			return ReadNum();
		}
		else if ( mCurrent == CHAR_VAR )  {
			return ReadVar();
		}
		else if ( mCurrent == CHAR_DQUOTE || mCurrent == CHAR_SQUOTE ) {
			return ReadStr();
		}
		else if ( isalpha( mCurrent )  ) {
			return ReadFunc();
		}
		else {
			return ReadOp();
		}
	}

	return ExprToken( ExprToken::etDone, "" );
}

//----------------------------------------------------------------------------
// step to next character in tokeniser input, setting current value
//----------------------------------------------------------------------------

char ExprTokeniser :: Next() {
	if ( mPos >= mExprs.size() ) {
		return mCurrent = 0;
	}
	mCurrent = mExprs[ mPos++ ];
	if ( mCurrent == '\n' ) {
		mLine++;
		mCol = 0;
	}
	else {
		mCol++;
	}
	return mCurrent;
}

//----------------------------------------------------------------------------
// One character lookahead into tokeniser input
//----------------------------------------------------------------------------

char ExprTokeniser :: Peek() const {
	return mPos >= mExprs.size() ? 0 : mExprs[ mPos ];
}


//----------------------------------------------------------------------------
// Skip over  non-significant whitespace, returning true if there is more
// input after the skipped spaces.
//----------------------------------------------------------------------------

bool ExprTokeniser :: ReadWS() {
	while( mCurrent ) {
		if ( isspace( mCurrent ) ) {
			Next();
		}
		else {
			return true;
		}
	}
	return false;
}

//----------------------------------------------------------------------------
// translate char which was escaped by backslash
//----------------------------------------------------------------------------

static char EscChr( char c ) {
	switch( c ) {
		case 'n':	return '\n';
		case 'r':	return '\r';
		case 't':	return '\t';
		default:	return c;
	}
}

//----------------------------------------------------------------------------
// Read string token. Must be called with current character being the first
// char in string, which  must be a  quote. Return string without quotes.
//----------------------------------------------------------------------------

ExprToken ExprTokeniser :: ReadStr() {
	string s;
	char quote = mCurrent;
	while(1) {
		Next();
		if ( mCurrent == CHAR_ESC ) {
			Next();
			if ( mCurrent == 0 ) {
				return Error( "Invalid escape" );
			}
			s += EscChr( mCurrent );
		}
		else if ( mCurrent == quote ) {
			Next();
			break;
		}
		else if ( mCurrent == 0 ) {
			return Error( "Unterminated string" );
		}
		s += mCurrent;
	}
	mCanBeUnaryMinus = false;
	return ExprToken( ExprToken::etStr, s );
}

//----------------------------------------------------------------------------
// Read function name, which must begin with and contain only alpha characters
// and end in an open paren with no spaces between the name and the paren.
// Returns name without paren.
//----------------------------------------------------------------------------

ExprToken ExprTokeniser :: ReadFunc() {
	string s;
	while( isalpha( mCurrent ) ) {
		s += mCurrent;
		Next();
	}

	mCanBeUnaryMinus = true;
	if ( mCurrent == '(' ) {
		Next();
		return ExprToken( ExprToken::etFunc, s );
	}
	else {
		return ExprToken( ExprToken::etStr, s );
	}
}

//----------------------------------------------------------------------------
// Variables must begin with '$', though this is not cheked here
// Rest of name can be alphanum or underscore. Returns name without '$'
//----------------------------------------------------------------------------

ExprToken ExprTokeniser :: ReadVar() {
	string name;
	bool isfunc = false;
	Next();					// skip $ sign already checked
	while(1) {
		name += mCurrent;
		Next();
		if ( mCurrent == '_' || isalnum( mCurrent ) ) {
			// ok - do nothing
		}
		else {
			break;
		}
	}
	if ( name.size() == 0 ) {
		return Error( "Unnamed variable" );
	}
	mCanBeUnaryMinus = false;
	return ExprToken( isfunc ? ExprToken::etFunc : ExprToken::etVar, name);
}

//----------------------------------------------------------------------------
// Read operator - all ops are 1 or 2 characters long
//----------------------------------------------------------------------------

ExprToken ExprTokeniser :: ReadOp() {
	unsigned int i = 0;

	if ( mCurrent == '-' && mCanBeUnaryMinus ) {
		Next();
		return ExprToken( ExprToken::etOp, UMINUS_STR, 20  );
	}

	while( const char * p = Ops[i].mOp ) {
		int pl = std::strlen(p);
		if ( pl == 1 && mCurrent == p[0] ) {
			Next();
			mCanBeUnaryMinus = * p == ')' ? false : true;
			return ExprToken( ExprToken::etOp, p, Ops[i].mPrec  );
		}
		else if ( pl == 2 && mCurrent == p[0] && Peek() == p[1] ) {
			Next();
			Next();
			mCanBeUnaryMinus = true;
			return ExprToken( ExprToken::etOp, p, Ops[i].mPrec  );
		}

		i++;
	}

	return Error( "Invalid operator " );
}

//----------------------------------------------------------------------------
// Read number. Numbers are always treated as reals.
//----------------------------------------------------------------------------

ExprToken ExprTokeniser :: ReadNum() {
	string num;
	bool havepoint = false;
	while(1) {
		if ( mCurrent == '.' ) {
			if ( ! havepoint ) {
				havepoint = true;
				if ( ! isdigit( Peek() ) ) {
					return Error( "Invalid number - expected digit" );
				}
			}
			else {
				return Error( "Invalid number - unexpected decimal point" );
			}
		}
		else if ( isdigit( mCurrent ) ) {
			// OK - do nothing
		}
		else if ( mCurrent == 0 || isspace( mCurrent )
					|| ! isalpha( mCurrent ) ) {
			if ( StrLast( num ) == '.' ) {
				return Error( "Invalid number - no digits following point" );
			}
			break;
		}
		else {
			return Error( "Invalid number" );
		}
		num += mCurrent;
		Next();
	}
	mCanBeUnaryMinus = false;
	return ExprToken( ExprToken::etNum, num );
}

//----------------------------------------------------------------------------
// Compiler takes tokens and turns them into a Reverse Polish form that
// is easy to execute. Uses a stack to handle operator precedence and
// sub-expressions.
//----------------------------------------------------------------------------

ExprCompiler :: ExprCompiler() {
}

ExprCompiler :: ~ExprCompiler() {
}

//----------------------------------------------------------------------------
// Sub expression is an expression in parens or a functiopn param list
// here we pop everything down to opening paren of function and add them to
// the Reverse Polish representation
//----------------------------------------------------------------------------

void ExprCompiler :: PopSubExpr( Expression::RPNRep & rep) {
	while( mStack.size() ) {
		ExprToken st = mStack.top();
		mStack.pop();
		//st.DumpOn( std::cout );
		if ( st.Type() == ExprToken::etOp &&  st.Value() == "("  ) {
			return;
		}
		else if ( st.Type() == ExprToken::etOp &&  st.Value() == FNCALL_STR ) {
			if ( mFuncStack.empty() ) {
				ATHROW( "Bad function call nesting" );
			}
			string fn = mFuncStack.top();
			mFuncStack.pop();
			rep.push_back( ExprToken( ExprToken::etStr, fn ) );
			rep.push_back( st );
			return;
		}
		else {
			rep.push_back( st );
		}
	}
	ATHROW( "Bad expression nesting or function call syntax" );
}

//----------------------------------------------------------------------------
// Handles precedence by popping everything of greater or equal prec
// and adding it to the RPN rep.
//----------------------------------------------------------------------------

void ExprCompiler :: PopHigherPrec( const ExprToken & tok,
									Expression::RPNRep & rep) {
	while( mStack.size() ) {
		ExprToken st = mStack.top();
		if ( st.Precedence() >= tok.Precedence() ) {
			rep.push_back( st );
			mStack.pop();
		}
		else {
			break;
		}
	}

	if ( tok == ExprToken::MakeSep() ) {
		rep.push_back( tok );
		Clear( mStack );
		// std::cout << "------\n";
	}
	else if ( tok.Value() != "," ) {
		mStack.push( tok );
	}
}

//----------------------------------------------------------------------------
// compile an expression into RPN, returning error message or
// the empty string if everthing was OK
//----------------------------------------------------------------------------

string ExprCompiler :: Compile( const string & expr,
								Expression::RPNRep & rep ) {

	Clear( mStack );
	Clear( mFuncStack );
	ExprTokeniser et( expr );

	while(1) {
		ExprToken tok = et.Get();
		//tok.DumpOn( std::cout );
		//std::cout << "\n";

		if ( tok.Type() == ExprToken::etDone ) {
			break;
		}
		else if ( tok.Type() == ExprToken::etError ) {
			return "Compilation error - " + tok.Value() ;
		}
		else if ( tok.Type() == ExprToken::etNum
					|| tok.Type() == ExprToken::etStr ) {
			rep.push_back( tok );
		}
		else if ( tok.Type() == ExprToken::etOp ) {
			if ( tok.Value() == ")" ) {
				PopSubExpr( rep );
			}
			else if ( tok.Value() == UMINUS_STR || tok.Value() == "(" ) {
				mStack.push( tok );
			}
			else {
				PopHigherPrec( tok, rep );
			}
		}
		else if ( tok.Type() == ExprToken::etFunc ) {
			mFuncStack.push( tok.Value() );
			ExprToken tmp( ExprToken::etOp, FNCALL_STR, 1 );
			mStack.push( tmp );
		}
		else if ( tok.Type() == ExprToken::etVar ) {
			rep.push_back( tok );
			ExprToken tmp( ExprToken::etOp, RDVAR_STR, 100 );
			mStack.push( tmp );
		}
		else {
			throw "Not supported yet";
		}
	}

	// add expr separator if user didn't bother to
	if ( rep.size() == 0 || ! (Last( rep ) == ExprToken::MakeSep()) ) {
		rep.push_back( ExprToken( ExprToken::etOp, EXPR_SEP, 0 ) );
	}

	return "";
}

//----------------------------------------------------------------------------
// Dump RPN representation for debug
//----------------------------------------------------------------------------

void ExprCompiler :: DumpRP( std::ostream & os, const Expression::RPNRep & rp ) {
	for ( unsigned int i = 0; i < rp.size(); i++ ) {
		os << "[";
		rp[i].DumpOn( os );
		os << "]";
	}
	os << "\n";
}

//----------------------------------------------------------------------------
// Expression uses compiler to compile string rep of expression into Reverse
// Polish form and then executes it. Alternatively, these stages can be
// performed separately.
//----------------------------------------------------------------------------

Expression :: Expression() {
}

Expression :: ~Expression() {
}


//----------------------------------------------------------------------------
// evaluate supplied expression, returning result as string
//----------------------------------------------------------------------------

string Expression :: Evaluate( const std::string & expr ) {
	string emsg = Compile( expr );
	if ( emsg != "" ) {
		ATHROW( emsg );
	}
	return Evaluate();
}

//----------------------------------------------------------------------------
// Evaluate pre-compiled expression
//----------------------------------------------------------------------------

string Expression :: Evaluate( ) {
	if ( mRPN.empty() ) {
		ATHROW( "No compiled expression" );
	}
	string result;
	unsigned int i = 0;
	//ExprCompiler::DumpRP( std::cout, mRPN );
	while( EvalSingleExpr( i, result ) ) {
	}
	return result;
}

//----------------------------------------------------------------------------
// compile to RP form, but don't evaluate expression
//----------------------------------------------------------------------------

string Expression :: Compile( const std::string & expr ) {
	string s = Trim( expr );
	if ( StrLast( s ) != EXPR_SEP[0] ) {
		s += EXPR_SEP;
	}
	mRPN.clear();
	ExprCompiler ec;
	string emsg = ec.Compile( s, mRPN );
	return emsg;
}

//----------------------------------------------------------------------------
// See if expression has compiled contents
//----------------------------------------------------------------------------

bool Expression :: IsCompiled() const {
	return mRPN.size() != 0;
}

//----------------------------------------------------------------------------
// Call named function, returning result of call.
// If anything goes wrong with popping the stack it almost certainly means
// the user didn't provide enough parameters.
//----------------------------------------------------------------------------

string Expression :: CallFunction( const string & name ) {
	const AddFunc * af = mFuncs.GetPtr( name );
	if ( af == 0 ) {
		ATHROW( "Unknown function: " << name );
	}
	std::deque <string> params;
	try {
		for ( unsigned int i = 0; i < af->mParamCount; i++ ) {
			params.push_front( PopStr() );
		}
	}
	catch( ... ) {
		ATHROW( "Function " << name << "() given the wrong number of parameters."
					<< " It takes "<< af->mParamCount << "." );
	}
	return af->mFunc( params, this );
}

//----------------------------------------------------------------------------
// Get positional parameter values
//----------------------------------------------------------------------------

unsigned int Expression :: PosParamCount() const {
	return mPosParams.size();
}

string Expression :: PosParam( unsigned int i ) const {
	return mPosParams.at( i );
}

//----------------------------------------------------------------------------
// add a positional parameter that can be accessed as $1, $2 etc.
//----------------------------------------------------------------------------

void Expression :: AddPosParam( const string & s ) {
	mPosParams.push_back( s );
}

//----------------------------------------------------------------------------
// clear all positional parameters
//----------------------------------------------------------------------------

void Expression :: ClearPosParams() {
	mPosParams.clear();
}

//----------------------------------------------------------------------------
// evaluate a single expression terminated by expression separator
//----------------------------------------------------------------------------

bool Expression :: EvalSingleExpr( unsigned int & ei, string & result ) {

	// do not remove this - we need to preserve previous result
	if ( ei >= mRPN.size() ) {
		return false;
	}

	result = "";
	Clear( mStack );

	while( ei < mRPN.size() ) {

		ExprToken tok = mRPN.at( ei++ );

		if ( tok == ExprToken::MakeSep() ) {
			if ( mStack.size() != 1 ) {
				ATHROW( "Invalid expression" );
			//	ATHROW( "Invalid expression in EvalSingleExpr stack "
			//				<< ei << " " << mStack.size() );
			}
			result = mStack.top();
			return true;
		}
		else if ( tok.Type() == ExprToken::etNum
					|| tok.Type() == ExprToken::etStr ) {
			mStack.push( tok.Value() );
		}
		else if ( tok.Type() == ExprToken::etOp ) {
			ExecOp( tok );
		}
		else if ( tok.Type() == ExprToken::etVar ) {
			mStack.push( tok.Value() );
		}
		else {
			tok.DumpOn( std::cerr );
			ATHROW( "Not implemented in EvalSingleExpr" );
		}
	}
	return false;
}

//----------------------------------------------------------------------------
// helper to pop string from stack or report error
//----------------------------------------------------------------------------

string Expression :: PopStr() {
	if ( mStack.size() == 0 ) {
		ATHROW( "Invalid expression" );
	}
	string s = mStack.top();
	mStack.pop();
	return s;
}

//----------------------------------------------------------------------------
// helper to pop number from stack
//----------------------------------------------------------------------------

double 	Expression ::  PopNum() {
	string s = PopStr();
	if ( ! IsNumber( s ) ) {
		ATHROW( "Invalid numeric value " << s );
	}
	return ToReal( s );
}

//----------------------------------------------------------------------------
// push boolean rep to stack
//----------------------------------------------------------------------------

void Expression :: PushBool( bool b ) {
	mStack.push( b ? "1" : "0"  );
}

//----------------------------------------------------------------------------
// handle comparison operators
//----------------------------------------------------------------------------

void Expression :: DoCompare( const string & op ) {

	string rhs = PopStr();
	string lhs = PopStr();

	if ( IsNumber( rhs ) && IsNumber( lhs ) ) {
		double dr = ToReal( rhs );
		double dl = ToReal( lhs );
		if ( op == "==" ) {
			PushBool( dl == dr );
		}
		else if ( op == "<>" || op == "!="  ) {		// we allow either
			PushBool( dl != dr );
		}
		else if ( op == "<" ) {
			PushBool( dl < dr );
		}
		else if ( op == ">" ) {
			PushBool( dl > dr );
		}
		else if ( op == ">=" ) {
			PushBool( dl >= dr );
		}
		else if ( op == "<=" ) {
			PushBool( dl <= dr );
		}
	}
	else {
		if ( op == "==" ) {
			PushBool( lhs == rhs );
		}
		else if ( op == "<>" || op == "!=" ) {
			PushBool( lhs != rhs );
		}
		else if ( op == "<" ) {
			PushBool( lhs < rhs );
		}
		else if ( op == ">" ) {
			PushBool( lhs > rhs );
		}
		else if ( op == ">=" ) {
			PushBool( lhs >= rhs );
		}
		else if ( op == "<=" ) {
			PushBool( lhs <= rhs );
		}
	}
}

//----------------------------------------------------------------------------
// convert string to bool - 0 or empty is false, anything else is true
//----------------------------------------------------------------------------

bool Expression :: ToBool( const std::string & s )  {
	if ( IsNumber(s ) ) {
		double d = ToReal( s );
		return d != 0.0;
	}
	else {
		return s != "";
	}
}

//----------------------------------------------------------------------------
// Handle the && and || operators. Currently we don't do short-circuited
// evaluation, but we certainly ought to.
//----------------------------------------------------------------------------

void Expression :: DoAndOr( const std::string & op ) {
	string rhs = PopStr();
	string lhs = PopStr();
	bool ok = ToBool( lhs );
	if ( ok ) {
		if ( op == "&&" ) {
			PushBool( ToBool( rhs ) );
		}
		else {
			PushBool( ok );
		}
	}
	else {
		if ( op == "||" ) {
			PushBool( ToBool( rhs ) );
		}
		else {
			PushBool( ok );
		}
	}
}

//----------------------------------------------------------------------------
// get value of variable. if the variable name is an integer it is a
// positional parameter otherwise it is a nmaed variable.
//----------------------------------------------------------------------------

string Expression :: GetVar( const string & var ) const {
	if ( IsInteger( var ) ) {
		int n = ToInteger( var ) - 1;
		if ( n < 0 ) {
			ATHROW( "Invalid positional parameter " << n );
		}
		if ( n >= (int) mPosParams.size() ) {
			return "";
		}
		else {
			return mPosParams[n];
		}
	}
	else {
		const string * val = mVars.GetPtr( var );
		if ( val == 0 ) {
			ATHROW( "Unknown variable: " << var );
		}
		return * val;
	}
}

//----------------------------------------------------------------------------
// clear all named variables
//----------------------------------------------------------------------------

void Expression :: ClearVars() {
	mVars.Clear();
}

//----------------------------------------------------------------------------
// add named variable, overwriting any exist ing value of same name
//----------------------------------------------------------------------------
void Expression :: AddVar( const string & name, const string & val ) {
	mVars.Replace( name, val );
}

//----------------------------------------------------------------------------
// Given a token containing an operator, execute it, pushing result to stack
//----------------------------------------------------------------------------

void Expression :: ExecOp( const ExprToken & tok ) {
	string op = tok.Value();
	if ( op == "." ) {
		string s = PopStr();
		mStack.push( PopStr() + s );
	}
	else if ( op == "+" ) {
		mStack.push( Str( PopNum() + PopNum() ) );
	}
	else if ( op == "-" ) {
		double n = PopNum();
		mStack.push( Str( PopNum() - n ) );
	}
	else if ( op == "*" ) {
		mStack.push( Str( PopNum() * PopNum() ) );
	}
	else if ( op == "/" ) {
		double n = PopNum();
		if ( n == 0 ) {
			ATHROW( "Divide by zero" );
		}
		mStack.push( Str( PopNum() / n ) );
	}
	else if ( op == "%" ) {
		double rhs = PopNum();
		double lhs = PopNum();
		if ( lhs < 0 || rhs < 0 ) {
			ATHROW( "Invalid operands for % operator" );
		}
		int n = int(lhs) % int(rhs);
		mStack.push( Str( n ) );
	}
	else if ( op == "*" ) {
		mStack.push( Str( PopNum() * PopNum() ) );
	}
	else if ( In( op, IgnoreCase, "==", "<>", "!=", "<", ">", "<=", ">=", NULL ) ) {
		DoCompare( op );
	}
	else if ( op == "&&" || op == "||" ) {
		DoAndOr( op );
	}
	else if ( op == UMINUS_STR ) {
		double d = - PopNum();
		mStack.push( Str( d ) );
	}
	else if ( op == RDVAR_STR ) {
		string s = PopStr();
		mStack.push( GetVar( s ) );
	}
	else if ( op == FNCALL_STR ) {
		string fun = PopStr();
		mStack.push( CallFunction( fun ) );
	}
	else {
		ATHROW( "Unknown operator: "  << op );
	}
}


//----------------------------------------------------------------------------

}	// namespace

//----------------------------------------------------------------------------
// Testing
//----------------------------------------------------------------------------

#ifdef ALIB_TEST
#include "a_myth.h"
using namespace ALib;
using namespace std;

DEFSUITE( "a_expr" );

DEFTEST( TokeniserTest1 ) {
	string e = "1 + 2";
	ExprTokeniser t( e );
	ExprToken tok = t.Get();
	FAILNE( tok.Value(), "1" );
	tok = t.Get();
	FAILNE( tok.Value(), "+" );
	tok = t.Get();
	FAILNE( tok.Value(), "2" );
	tok = t.Get();
	FAILNE( tok.Type(), ExprToken::etDone );

}

DEFTEST( CompilerTest ) {
	string expr = " 1 + 2 * 6;";
	ExprCompiler cp;
	Expression::RPNRep rp;
	string r = cp.Compile( expr, rp );
//	ExprCompiler::DumpRP( cout, rp );
	FAILNE( r, "" );
	FAILNE( rp.size(), 6 );

	expr = "-2;";
	rp.clear();
	r = cp.Compile( expr, rp );
	FAILNE( r, "" );
	FAILNE( rp.size(), 3 );
	//ExprCompiler::DumpRP( cout, rp );
	expr = "$1 + 2;";
	rp.clear();
	r = cp.Compile( expr, rp );
	//ExprCompiler::DumpRP( cout, rp );
	FAILNE( r, "" );
	FAILNE( rp.size(), 5 );

	expr = "(1 + 2) *3;";
	rp.clear();
	r = cp.Compile( expr, rp );
	//ExprCompiler::DumpRP( cout, rp );
	FAILNE( r, "" );
	FAILNE( rp.size(), 6 );

	expr = "1 == 2;";
	rp.clear();
	r = cp.Compile( expr, rp );
	//ExprCompiler::DumpRP( cout, rp );
	FAILNE( r, "" );
	FAILNE( rp.size(), 4 );

	expr = "5 + inc(3);" ;
	rp.clear();
	r = cp.Compile( expr, rp );
	//ExprCompiler::DumpRP( cout, rp );

}

DEFTEST( PosParamTest ) {
	Expression e;
	e.AddPosParam( "foo" );
	e.AddPosParam( "bar" );
	FAILNE( e.PosParamCount(), 2 );
	FAILNE( e.PosParam(0), "foo" );
	FAILNE( e.PosParam(1), "bar" );

	// these use pos params
	string s = e.Evaluate( "find('bar')" );
	FAILNE( s, "2" );
	s = e.Evaluate( "find('zod')" );
	FAILNE( s, "0" );

}

DEFTEST( ExpressionTest ) {
	Expression e;
	string s = e.Evaluate( "1 + 2;" );
	FAILNE( s, "3" );
	s = e.Evaluate( "2 * 5;" );
	FAILNE( s, "10" );
	s = e.Evaluate( "10 / 2" );
	FAILNE( s, "5" );
	s = e.Evaluate( "1;2;" );
	FAILNE( s, "2" );
	s = e.Evaluate( "3-1" );
	FAILNE( s, "2" );
}

DEFTEST( NumTest ) {
	Expression e;
	string s = e.Evaluate( "isint('42');" );
	FAILNE( s, "1" );
	s = e.Evaluate( "isint('42.0');" );
	FAILNE( s, "0" );
}

DEFTEST( UnaryMinusTest ) {
	Expression e;
	string s = e.Evaluate( "-2;" );
	FAILNE( s, "-2" );
	s = e.Evaluate( "1--2;" );
	FAILNE( s, "3" );
}

DEFTEST( VarTest ) {
	Expression e;
	e.AddPosParam( "3" );
	string s = e.Evaluate( "$1;" );
	FAILNE( s, "3" );
	s = e.Evaluate( "$1+2;" );
	FAILNE( s, "5" );
	e.AddVar( "beast", "666" );
	s = e.Evaluate( "$beast;" );
	FAILNE( s, "666" );
	s = e.Evaluate( "field(1)" );
	FAILNE( s, "3" );
}

DEFTEST( ParenTest ) {
	Expression e;
	string s = e.Evaluate( "(1 + 2) * 3" );
	FAILNE( s, "9" );
}

DEFTEST( BoolTest ) {
	Expression e;
	string s = e.Evaluate( "1 == 2;" );
	FAILNE( s, "0" );
	s = e.Evaluate( "1 <> 2;" );
	FAILNE( s, "1" );
	s = e.Evaluate( "1 != 2;" );
	FAILNE( s, "1" );
	s = e.Evaluate( "1 && 2;" );
	FAILNE( s, "1" );
	s = e.Evaluate( "1 && 0;" );
	FAILNE( s, "0" );
	s = e.Evaluate( "1 || 0;" );
	FAILNE( s, "1" );
	s = e.Evaluate( "1 < 2" );
	FAILNE( s, "1" );
	s = e.Evaluate( "2 > 1" );
	FAILNE( s, "1" );
	s = e.Evaluate( "1 <= 2" );
	FAILNE( s, "1" );
	s = e.Evaluate( "2 >= 1" );
	FAILNE( s, "1" );
}

DEFTEST( IsEmptyTest ) {
	Expression e;
	string s = e.Evaluate( "isempty('')" );
	FAILNE( s, "1" );
	s = e.Evaluate( "isempty('foo')" );
	FAILNE( s, "0" );
	s = e.Evaluate( "isempty(' \t')" );
	FAILNE( s, "1" );
}

DEFTEST( RegExTest ) {
	Expression e;
	string s = e.Evaluate( "match('','^$' )" );
	FAILNE( s, "1" );
	s = e.Evaluate( "match('foo','^$')" );
	FAILNE( s, "0" );
	s = e.Evaluate( "match('foo','foo')" );
	FAILNE( s, "1" );
	s = e.Evaluate( "match('foo','f.*')" );
	FAILNE( s, "1" );
}

struct ExprTest {
	const char * expr;
	const char * result;
};

static ExprTest ETests[] = {
	{"\"\"","" },
	{"\"xxx\"","xxx" },
	{"0", "0"},
	{"1", "1"},
	{"1 + 2 * 3", "7" },
	{"(1  + 2) * 3", "9" },
	{"9 % 2", "1" },
	{"1 + 3 * ((2  * 10 + 5) / 5)", "16" },
	{"not(1)", "0"},
	{"if( 1, 'foo', 'bar')", "foo"},
	{"if( 0, 'foo', 'bar')", "bar"},
	{"int(123.456)", "123"},
	{"substr('1234567890', 2, 3)", "234"},
	{"sign(-42)", "-1"},
	{"abs(-42)", "42"},
	{"trim('  foo ')", "foo"},
	{"pos('foobar','ob')", "3"},
	{"foo . bar", "foobar"},
	{"lower('FOO')", "foo" },
	{"upper('foo')", "FOO" },
	{"len('foo')", "3" },
	{"streq('foo','FOO')", "1" },
	{"streq('foox','FOO')", "0" },
	{"min('3','6')", "3" },
	{"max('3','6')", "6" },
	{"day('1980-10-7')", "7" },
	{"month('1980-10-7')", "10" },
	{"year('1980-10-7')", "1980" },
	{"isdate('1980-10-7')", "1" },
	{"isdate('1980-13-7')", "0" },
	{"index('two', 'one,two,three' )", "2" },
	{"pick('2', 'one,two,three' )", "two" },
	{NULL,NULL}
};

DEFTEST( AllExprs ) {
	int i = 0;
	while( ETests[i].expr ) {
		Expression e;
		string s = e.Evaluate( ETests[i].expr );
		string si = Str(i);
		FAILNEM( s, ETests[i].result, (si + string(": ") + ETests[i].expr ));
		i++;
	}
}

#endif

// end

