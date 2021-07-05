//---------------------------------------------------------------------------
// csved_rules.h
//
// rules for csvfix validation
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#ifndef INC_CSVED_RULES_H
#define INC_CSVED_RULES_H

#include "a_base.h"
#include "a_str.h"
#include "a_dict.h"
#include "a_date.h"
#include "csved_types.h"
#include "csved_util.h"
#include "csved_date.h"
#include <limits.h>
#include <float.h>
#include <set>

namespace CSVED {

//---------------------------------------------------------------------------
// These are the rule names
//---------------------------------------------------------------------------

const std::string RULE_REQD			= "required";
const std::string RULE_NOTEMPTY		= "notempty";
const std::string RULE_NUM			= "numeric";
const std::string RULE_VALUES		= "values";
const std::string RULE_NOTVALUES	= "notvalues";
const std::string RULE_LOOKUP		= "lookup";
const std::string RULE_FIELDS		= "fields";
const std::string RULE_DATE			= "date";
const std::string RULE_LENGTH		= "length";


//---------------------------------------------------------------------------
// Class used to store validation failure result info
//---------------------------------------------------------------------------

class ValidationResult {

	public:

		ValidationResult()
			: mOk( true ), mField( 0 ) {
		}

		ValidationResult( int f, const std::string & msg )
					: mOk( false ), mField( f + 1 ), mMsg( msg ) {
		}

		bool OK() const {
			return mOk;
		}

		int Field() const {
			return mField;
		}

		std::string Msg() const {
			return mMsg;
		}

	private:

		bool mOk;
		int mField;
		std::string mMsg;

};

//---------------------------------------------------------------------------
// Abstract base from which all other rules are derived.
//---------------------------------------------------------------------------

class ValidationRule {

	public:

		typedef std::vector <ValidationResult> Results;
		typedef std::vector <std::string> Params;

		ValidationRule( const std::string & name,
						const FieldList & fl,
						const Params &  );

		virtual ~ValidationRule();

		virtual Results Apply( const CSVRow & row ) const;

		std::string Name() const;
		void DumpOn( std::ostream & os );

	protected:

		virtual ValidationResult Validate( const CSVRow & row,
											unsigned int idx ) const = 0 ;

		unsigned int FieldCount() const {
			return mFields.size();
		}

		unsigned int FieldAt( unsigned int i ) const {
			return mFields.at(i);
		}

		void NeedFields() const;

	private:

		std::string mName;
		FieldList mFields;
};

//---------------------------------------------------------------------------
// Factory for creating rules given a rule name. Rules are registered
// with the factory via the templated AddRule class.
//---------------------------------------------------------------------------

class RuleFactory {

	public:

		typedef ValidationRule *
			(* CreateFunc) ( const std::string & name,
							const FieldList & fl,
							 const ValidationRule::Params & params );

		static void AddCreateFunc( const std::string & name, CreateFunc func );

		static ValidationRule * CreateRule( const std::string & name,
											const FieldList & fl,
											const ValidationRule::Params & params );
	private:

		static void InitDict();

		typedef ALib::Dictionary <CreateFunc> DictType;
		static DictType * mDict;


};

//---------------------------------------------------------------------------
// Template to add rule to factory
//---------------------------------------------------------------------------

template <typename RT> struct AddRule {

	static ValidationRule * Create( const std::string & name,
									 const FieldList & fl,
									 const ValidationRule::Params & params ) {
		return new RT( name, fl, params );
	}

	AddRule( const std::string & name ) {
		RuleFactory::AddCreateFunc( name, Create );
	}
};

//---------------------------------------------------------------------------
// Required rule says that an indexed field must exist in CSV row - it
// may however contain an empty string
//---------------------------------------------------------------------------

class RequiredRule : public ValidationRule {

	public:

		RequiredRule( const std::string & name,
						const FieldList & fl,
						const Params & params );

		ValidationResult Validate( const CSVRow & row,
									unsigned int idx ) const ;

};

//---------------------------------------------------------------------------
// Not empty says that if field exists itn must not contain only whitespace.
//---------------------------------------------------------------------------

class NotEmptyRule : public ValidationRule {

	public:

		NotEmptyRule( const std::string & name,
						const FieldList & fl,
						const Params & params );

		ValidationResult Validate( const CSVRow & row,
									unsigned int idx ) const ;

};

//---------------------------------------------------------------------------
// Say value must be in list of values
//---------------------------------------------------------------------------

class ValuesRule : public ValidationRule {

	public:

		ValuesRule( const std::string & name,
						const FieldList & fl,
						const Params & params );

		ValidationResult Validate( const CSVRow & row,
									unsigned int idx ) const ;

	private:

		Params mValues;

};

//---------------------------------------------------------------------------
// Require minimum & maximum number of fields
//---------------------------------------------------------------------------


class FieldsRule : public ValidationRule {

	public:

		FieldsRule( const std::string & name,
						const FieldList & fl,
						const Params & params );

		virtual Results Apply( const CSVRow & row ) const;

		ValidationResult Validate( const CSVRow & row,
									unsigned int idx ) const ;

	private:

		unsigned int mMin, mMax;	// number of fields

};

//----------------------------------------------------------------------------
// Require field length is in range
//----------------------------------------------------------------------------

class LengthRule : public ValidationRule {

	public:

		LengthRule( const std::string & name,
						const FieldList & fl,
						const Params & params );

		ValidationResult Validate( const CSVRow & row,
									unsigned int idx ) const ;

	private:

		int mMin, mMax;	// field length

};

//---------------------------------------------------------------------------
// Numeric rule says field must be numeric and optionally be in a range
//---------------------------------------------------------------------------

class NumericRule : public ValidationRule {

	public:

		NumericRule( const std::string & name,
						const FieldList & fl,
						const Params & params );

		ValidationResult Validate( const CSVRow & row,
									unsigned int idx ) const ;

	private:

		ValidationResult ValidateRanges( const CSVRow & row,
									unsigned int idx ) const ;

		void MakeRanges( const Params & params );

		typedef std::pair <double,double> Range;
		typedef std::vector <Range> Ranges;
		Ranges mRanges;
};

//---------------------------------------------------------------------------
// Check that field values exist in another CSV file
//---------------------------------------------------------------------------

class LookupRule : public ValidationRule {

	public:

		LookupRule( const std::string & name,
						const FieldList & fl,
						const Params & params );

		ValidationResult Validate( const CSVRow & row,
									unsigned int idx ) const ;
		virtual Results Apply( const CSVRow & row ) const;

	private:

		void Init( const Params & params );
		void BuildFieldSet();

		typedef std::pair <unsigned int ,unsigned int> Join;
		typedef std::vector <Join> Joins;
		Joins mJoins;

		std::string mLookupFile;

		std::multiset <std::string> mJoinVals;
};

//---------------------------------------------------------------------------
// Validate that a field is a date and that the date is within a range
//---------------------------------------------------------------------------

class DateRule : public ValidationRule {

	public:

		DateRule( const std::string & name,
						const FieldList & fl,
						const Params & params );

		~DateRule();

		ValidationResult Validate( const CSVRow & row,
									unsigned int idx ) const ;

	private:

		bool mCheckRange;
		std::string mMask;
		ALib::Date mMin, mMax;
		mutable MaskedDateReader * mDateReader;
};

//------------------------------------------------------------------------

}	// end namespace

#endif

