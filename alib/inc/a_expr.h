//----------------------------------------------------------------------------
// a_expr.h
//
// simple expression eveluation
//
// Copyright (C) 2009 Neil Butterworth
//----------------------------------------------------------------------------

#ifndef INC_A_EXPR_H
#define INC_A_EXPR_H

#include "a_base.h"
#include "a_dict.h"
#include <stack>
#include <iosfwd>
#include <deque>

namespace ALib {

//----------------------------------------------------------------------------
// Tokens have type and value (and prcedence for operator tokens)
//----------------------------------------------------------------------------

class ExprToken {

	public:

		enum ETType {
			etNone, etOp, etNum, etStr, etVar, etFunc, etError, etDone
		};

		ExprToken() : mType( etNone ), mValue( "" ), mPrec(-1) {}

		ExprToken( ETType type, const std::string & value, int prec = -1 )
			: mType( type ), mValue( value ), mPrec( prec ) {}

		ETType Type() const {
			return mType;
		}

		const std::string & Value() const {
			return mValue;
		}

		int Precedence() const {
			return mPrec;
		}

		bool IsSep() const;
		static ExprToken MakeSep();
		bool operator == ( const ExprToken & et ) const;

		void DumpOn( std::ostream & os ) const;

	private:

		ETType mType;
		std::string mValue;
		int mPrec;
};

//----------------------------------------------------------------------------
/// Simple arithmetic & string expression evaluator
//----------------------------------------------------------------------------

class Expression {

	public:

		// reverse polish representation
		typedef std::vector <ExprToken> RPNRep;

		// type for functions within expression
		typedef std::string (*FuncImpl)( const std::deque <std::string> &,
											Expression * );

		Expression();
		~Expression();

		std::string Compile( const std::string & expr );
		bool IsCompiled() const;

		std::string Evaluate();
		std::string Evaluate( const std::string & expr );

		void ClearPosParams();
		void AddPosParam( const std::string & s );

		unsigned int PosParamCount() const;
		std::string PosParam( unsigned int i ) const;

		void ClearVars();
		void AddVar( const std::string & name, const std::string & val );

		struct AddFunc {
			FuncImpl mFunc;
			unsigned int mParamCount;

			AddFunc( const std::string & name, FuncImpl fi, unsigned int np )
				: mFunc( fi ), mParamCount( np ) {
				Expression::AddFunction( name, * this );
			}
		};

		static void AddFunction( const std::string & name, const AddFunc & f );
		static bool ToBool( const std::string & s );

		static void SetRNGSeed( int n );
		static int GetRNGSeed();

	private:

		std::string PopStr();
		double PopNum();
		std::string GetVar( const std::string & var ) const;
		void DoCompare( const std::string & op );
		void DoAndOr( const std::string & op );
		void PushBool( bool b );
		void ExecOp( const ExprToken & tok );
		bool EvalSingleExpr( unsigned int & ei, std::string & result );
		std::string CallFunction( const std::string & name );

		std::vector <std::string> mPosParams;
		std::stack <std::string> mStack;
		Dictionary <std::string> mVars;

		RPNRep mRPN;

		static Dictionary <AddFunc> mFuncs;

		static bool mUseRNGSeed;
		static int mRNGSeed;

};

//----------------------------------------------------------------------------

}	// namespace

#endif
