//---------------------------------------------------------------------------
// csved_rules.cpp
//
// rules for validation
//
// Copyright (C) 2009 Neil Butterworth
//---------------------------------------------------------------------------

#include "a_base.h"
#include "a_str.h"
#include "a_collect.h"
#include "csved_rules.h"
#include "csved_except.h"
#include <fstream>

using std::string;
using std::vector;

namespace CSVED {

//---------------------------------------------------------------------------
// Static pointer to single rule factory dictionary.
// ??? maybe use single factory instaed - dict is used for historic reasons
//---------------------------------------------------------------------------

RuleFactory::DictType * RuleFactory::mDict = 0;

//---------------------------------------------------------------------------
// Initialise dictionary if not aalready initialised
//---------------------------------------------------------------------------

void RuleFactory :: InitDict() {
	static bool needinit = true;
	if ( needinit ) {
		mDict = new DictType;
		needinit = false;
	}
}

//---------------------------------------------------------------------------
// Add a name/func pointer pair to dictionary. The FP must point to a
// function that can create instances of the named rule.
//---------------------------------------------------------------------------

void RuleFactory :: AddCreateFunc( const string & name, CreateFunc cf ) {
	InitDict();
	if ( mDict->Contains( name ) ) {
		CSVTHROW( "Duplicate rule name: " << name );
	}
	mDict->Add( name, cf );
}

//---------------------------------------------------------------------------
// Create rule by looking FP up by name and then calling it.
// Returns false if no such rule.
//---------------------------------------------------------------------------

ValidationRule * RuleFactory ::  CreateRule( const string & name,
								const FieldList & fl,
								const ValidationRule::Params & params ) {
	InitDict();
	if ( ! mDict->Contains( name ) ) {
		return NULL;
	}
	else {
		CreateFunc cf = mDict->Get( name );
		return cf( name, fl, params );
	}
}

//------------------------------------------------------------------------
// Abstract base for all rules
//---------------------------------------------------------------------------

ValidationRule :: ValidationRule( const string & name,
									const FieldList & fl,
									const Params &  )
	: mName( name ), mFields( fl ) {
}

ValidationRule :: ~ValidationRule() {
	// nothing
}

string ValidationRule :: Name() const {
	return mName;
}

void ValidationRule :: NeedFields() const {
	if ( mFields.size() == 0 ) {
		CSVTHROW( Name() << " needs field list" );
	}
}

//---------------------------------------------------------------------------
// Loop over all fields for this rule, calling validate on each. Store
// any error results and return them.
//---------------------------------------------------------------------------


ValidationRule::Results ValidationRule :: Apply(
										const CSVRow & row ) const {
	Results r;
	for ( unsigned int i = 0; i < mFields.size(); i++ ) {
		ValidationResult res = Validate( row, mFields[i] );
		if ( ! res.OK() ) {
			r.push_back( res );
		}
	}
	return r;
}

//---------------------------------------------------------------------------
// Debug dump
//---------------------------------------------------------------------------

void ValidationRule ::DumpOn( std::ostream & os ) {
	os << "Rule:" << Name() <<"\n";
	os << "Fields:: " << "\n";
	ALib::Dump( os, mFields );
	os << "---------------------\n";
}

//----------------------------------------------------------------------------
// Helper to split a string in the form "num1:num2" into two numeric values
//----------------------------------------------------------------------------

void SplitRange( const string & s, int & a, int & b ) {

	vector <string> tmp;

	if ( ALib::Split( s, ':', tmp ) != 2 ) {
		CSVTHROW( "Need range in form n1:n2, not " << s );
	}

	if ( ! ALib::IsInteger( tmp[0] ) || ! ALib::IsInteger( tmp[1] ) ) {
		CSVTHROW( "Invalid number in range:" << s );
	}

	a = ALib::ToInteger( tmp[0] );
	b = ALib::ToInteger( tmp[1] );
}

//---------------------------------------------------------------------------
// Required says that column must exist in row
//---------------------------------------------------------------------------

static AddRule <RequiredRule> r1_( RULE_REQD );		// add rule to factory


RequiredRule :: RequiredRule( const string & name,
								const FieldList & fl,
								const Params & params	 )
	: ValidationRule( name, fl, params ) {
	NeedFields();
}


ValidationResult RequiredRule :: Validate( const CSVRow & row,
											unsigned int idx ) const {
	if ( idx >= row.size() ) {
		return ValidationResult( idx, "required field missing" );
	}
	else {
		return ValidationResult();
	}
}

//---------------------------------------------------------------------------
// Not empty says that if field exists itn must not contain only whitespace.
//---------------------------------------------------------------------------

static AddRule <NotEmptyRule> r2_( RULE_NOTEMPTY );	// add rule to factory

NotEmptyRule :: NotEmptyRule( const string & name,
								const FieldList & fl,
								const Params & params )
	: ValidationRule( name, fl, params ) {
	NeedFields();
}

ValidationResult NotEmptyRule :: Validate( const CSVRow & row,
											unsigned int idx ) const {
	if ( idx < row.size() && ALib::IsEmpty( row[idx] ) ) {
		return ValidationResult( idx, "field is empty" );
	}
	else {
		return ValidationResult();
	}
}



//---------------------------------------------------------------------------
// Numeric tests if a value is numeric and optionaly in a range
//---------------------------------------------------------------------------

static AddRule <NumericRule> r3_( RULE_NUM );		// add rule to factory

NumericRule :: NumericRule( const string & name,
							const FieldList & fl,
							const Params & params )
	: ValidationRule( name, fl, params ) {
	NeedFields();
	MakeRanges( params );
}

//---------------------------------------------------------------------------
// Check we have a number and if so if it is in range
//---------------------------------------------------------------------------

ValidationResult NumericRule :: Validate( const CSVRow & row,
											unsigned int idx ) const {

	if ( idx < row.size() ) {
		string val = row[idx];
		if ( ! ALib::IsNumber( val ) ) {
			return ValidationResult( idx, ALib::DQuote(val)
											+ " is not numeric" );
		}
		else {
			return ValidateRanges( row, idx );
		}
	}
	return ValidationResult();
}

//---------------------------------------------------------------------------
// Build ranges from parameters. Rangers are specified by min::max pairs
//---------------------------------------------------------------------------

void NumericRule :: MakeRanges( const Params & params ) {
	for ( unsigned int i = 0; i < params.size(); i++ ) {
		vector <string> tmp;
		if ( ALib::Split( params[i], ':', tmp ) != 2
				|| ! ALib::IsNumber( tmp[0] )
				|| ! ALib::IsNumber( tmp[1] ) ) {
			CSVTHROW( "Invalid numeric range:" << params[i] );
		}
		double min = ALib::ToReal( tmp[0] );
		double max = ALib::ToReal( tmp[1] );
		if ( min > max ) {
			CSVTHROW( "Invalid numeric range:" << params[i] );
		}
		mRanges.push_back( std::make_pair( min, max ) );
	}
}

//---------------------------------------------------------------------------
// If we have any ranges, check that value being validated falls within
// one of them.
//---------------------------------------------------------------------------

ValidationResult NumericRule ::ValidateRanges( const CSVRow & row,
												unsigned int idx ) const {
	if ( mRanges.size() == 0 ) {
		return ValidationResult();
	}

	double n = ALib::ToReal( row[idx] );
	bool ok = false;
	for ( unsigned int i = 0; i < mRanges.size(); i++ ) {
		if ( n >= mRanges[i].first && n <= mRanges[i].second ) {
			ok = true;
			break;
		}
	}

	return ok ? ValidationResult()
			  : ValidationResult( idx, ALib::DQuote(row[idx])
										+ " failed range check" );
}

//---------------------------------------------------------------------------
// Rule to check the number of fields is within a range
//---------------------------------------------------------------------------

static AddRule <FieldsRule> FieldsRule1_( RULE_FIELDS );

FieldsRule :: FieldsRule( const string & name,
								const FieldList & fl,
								const Params & params )
	: ValidationRule( name, fl, params )  {

	// don't need fields - do need value
	if ( params.size() == 0 ) {
		CSVTHROW( "Rule " << name << " needs values" );
	}
	else if ( params.size() > 1 ) {
		CSVTHROW( "Rule " << name << " needs min,max values only" );
	}

	vector <string> tmp;
	if ( ALib::Split( params[0], ':', tmp )  != 2 ) {
		CSVTHROW( "Rule " << name << " needs min,max values" );
	}
	if ( ! ALib::IsInteger( tmp[0] ) || ! ALib::IsInteger( tmp[1] ) ) {
		CSVTHROW( "Rule " << name << " needs min,max values as integers" );
	}

	int imin = ALib::ToInteger( tmp[0] );
	int imax = ALib::ToInteger(  tmp[1] );

	if ( imin < 1 || imin > imax || imax < imin ) {
		CSVTHROW( "Rule " << name << " has invalid min,max values " );
	}

	mMin = imin;
	mMax = imax;
}

//---------------------------------------------------------------------------
// Never called - all work done in  apply
//---------------------------------------------------------------------------

ValidationResult FieldsRule :: Validate( const CSVRow &,
										unsigned int ) const {
	return ValidationResult();
}

//---------------------------------------------------------------------------
// Check number of fields is in min/mac range
//---------------------------------------------------------------------------

ValidationRule::Results FieldsRule :: Apply( const CSVRow & row ) const {
	unsigned int n = row.size();
	Results res;
	if ( n < mMin ) {
		res.push_back( ValidationResult( 0, "Not enough fields" ) );
	}
	else if ( n > mMax ) {
		res.push_back(  ValidationResult( 0, "Too many fields" ) );
	}
	return res;
}


//---------------------------------------------------------------------------
// Values tests a field againsta  list of values
//---------------------------------------------------------------------------

static AddRule <ValuesRule> r4_( RULE_VALUES );		// use same rule for values
static AddRule <ValuesRule> r5_( RULE_NOTVALUES );	// and notvalues


ValuesRule :: ValuesRule( const string & name,
								const FieldList & fl,
								const Params & params )
	: ValidationRule( name, fl, params ) {

	NeedFields();
	if ( params.size() == 0 ) {
		CSVTHROW( "Rule " << name << " needs values" );
	}

	for( unsigned int i = 0; i < params.size(); i++ ) {
		mValues.push_back( params[i] );
	}
}

//---------------------------------------------------------------------------
// Test against list o values. Invert sense of test if 'notvalues' rule.
//---------------------------------------------------------------------------

ValidationResult ValuesRule :: Validate( const CSVRow & row,
									unsigned int idx ) const  {
	if ( idx >= row.size() ) {
		return ValidationResult();
	}

	string val = row[idx];
	unsigned int matches = 0;
	for ( unsigned int i = 0; i < mValues.size(); i++ ) {
		if ( val == mValues[i] ) {
			matches++;
		}
	}

	if ( Name() == RULE_VALUES ) {
		return matches > 0
				? ValidationResult()
				: ValidationResult( idx, ALib::DQuote(val)
										+ " is invalid value" );
	}
	else {
		return matches == 0
				? ValidationResult()
				: ValidationResult( idx, ALib::DQuote(val)
										+ " is invalid value" );
	}
}


//---------------------------------------------------------------------------
// Length rule tests field lebgth is in ramge
//---------------------------------------------------------------------------

static AddRule <LengthRule> lr1_( RULE_LENGTH );

LengthRule :: LengthRule( const string & name,
								const FieldList & fl,
								const Params & params )
	: ValidationRule( name, fl, params )  {

	NeedFields();
	if ( params.size() == 0 ) {
		CSVTHROW( "Rule " << name << " needs values" );
	}
	SplitRange( params[0], mMin, mMax );
	if ( mMin < 0 || mMin > mMax ) {
		CSVTHROW( "Invalid range " << params[0] << " in rule " << name );
	}
}

ValidationResult LengthRule :: Validate( const CSVRow & row,
									unsigned int idx ) const  {
	if ( idx >= row.size() ) {
		return ValidationResult();
	}

	string val = row[idx];
	int len = val.size();

	if ( len < mMin ) {
		return 	ValidationResult( idx, ALib::DQuote(val) + " is too short" );
	}
	else if ( len > mMax ) {
		return 	ValidationResult( idx, ALib::DQuote(val) + " is too long" );
	}
	else {
		return ValidationResult();
	}
}

//---------------------------------------------------------------------------
// lookup rule validates fields against fields in a lookup file
//---------------------------------------------------------------------------

static AddRule <LookupRule> r6_( RULE_LOOKUP );

LookupRule :: LookupRule( const string & name,
							const FieldList & fl,
							const Params & params )
	: ValidationRule( name, fl, params ) {
	Init( params );
	BuildFieldSet();
}

//---------------------------------------------------------------------------
// Helper to create displayable version of key
//---------------------------------------------------------------------------

static string DisplayKey( const std::string & key ) {
	string d;
	for ( unsigned int i = 0; i < key.size(); i++ ) {
		if ( key[i] == 0 ) {
			if ( i != key.size() - 1 ) {
				d += "|";
			}
		}
		else {
			d += key[i];
		}
	}
	return ALib::SQuote( d );
}

//---------------------------------------------------------------------------
// Lookup needs to look at multiple field values at once, so we override
// Apply() rather than using Validate(). Build a key and then lookup in
// the set of key values. If lookup fails, report error but remember we
// don't have field list so use -1 for field number
//---------------------------------------------------------------------------

ValidationRule::Results LookupRule :: Apply( const CSVRow & row ) const {

	string key;

	for ( unsigned int i = 0; i < mJoins.size(); i++ ) {
		unsigned int fi = mJoins[i].first;
		if ( fi < row.size() ) {
			key += row[fi];
		}
		key += '\0';
	}

	Results res;
	if ( mJoinVals.find( key ) == mJoinVals.end() ) {
		res.push_back(
			ValidationResult( -1, "lookup of " + DisplayKey( key )
								+ " in " + mLookupFile + " failed" )
		);
	}
	return res;
}


//---------------------------------------------------------------------------
// This is never called - all work done in Apply()
//---------------------------------------------------------------------------

ValidationResult LookupRule :: Validate( const CSVRow &,
											unsigned int  ) const {
	return ValidationResult();
}

//---------------------------------------------------------------------------
// Set up the lookup filename & build the key field pair list
//---------------------------------------------------------------------------

void LookupRule :: Init( const Params & params ) {

	if ( params.size() != 2 ) {
		CSVTHROW( Name() << " needs field list and filename" );
	}

	mLookupFile = params[1];
	mJoins.clear();
	ALib::CommaList cl( params[0] );
	for ( unsigned int i = 0; i < cl.Size(); i++ ) {
		vector <string> tmp;
		if ( ALib::Split( cl.At(i), ':', tmp ) != 2 ) {
			CSVTHROW( "Invalid field list entry: " << cl.At(i) );
		}
		if ( ! ALib::IsInteger( tmp[0] ) || ! ALib::IsInteger( tmp[1] ) ) {
			CSVTHROW( "Invalid field list entry: " << cl.At(i) );
		}
		int f1 = ALib::ToInteger( tmp[0] ) - 1;
		int f2 = ALib::ToInteger( tmp[1] ) - 1;
		if ( f1 < 0 || f2 < 0 ) {
			CSVTHROW( "Invalid field list entry: " << cl.At(i) );
		}
		mJoins.push_back(
			std::make_pair( (unsigned int) f1, (unsigned int) f2 )
		);
	}
}

//---------------------------------------------------------------------------
// Read all of the lokup file and build set of key field values to use
// to perform the actual validation.
//---------------------------------------------------------------------------

void LookupRule :: BuildFieldSet() {

	std::ifstream ifs( mLookupFile.c_str() );
	if ( ! ifs.is_open() ) {
		CSVTHROW( "Cannot open lookup file " << mLookupFile << " for input" );
	}

	ALib::CSVStreamParser p( ifs );
	CSVRow row;

	while( p.ParseNext( row ) ) {
		string key;
		for ( unsigned int i = 0; i < mJoins.size(); i++ ) {
			unsigned int jf = mJoins[i].second;
			if ( jf < row.size() ) {
				key += row[jf];
			}
			key += '\0';
		}
		mJoinVals.insert( key );
	}
	ifs.close();
}


//---------------------------------------------------------------------------
// Validate daate against mask and optionally check range
//---------------------------------------------------------------------------

static AddRule <DateRule> DateRule1_( RULE_DATE );

//---------------------------------------------------------------------------
// Check we have a date mask and opyionally a date range
//---------------------------------------------------------------------------

DateRule :: DateRule( const string & name,
						const FieldList & fl,
						const Params & params )
	: ValidationRule( name, fl, params ), mDateReader( 0 ) {

	NeedFields();

	if ( params.size() == 0 ) {
		CSVTHROW( "Rule " << name << " needs date mask" );
	}
	mMask = params[0];

	if ( params.size() > 2 ) {
		CSVTHROW( "Rule " << name << " needs mask and range parameters only" );
	}

	if ( params.size() == 2 ) {
		vector <string> tmp;
		if ( ALib::Split( params[1], ':', tmp ) != 2 ) {
			CSVTHROW( "Rule " << name << " has invalid range: " << params[1] );
		}

		if ( ALib::Date::Validate( tmp[0] ) != ALib::Date::DATEOK
				|| ALib::Date::Validate( tmp[1] ) != ALib::Date::DATEOK ) {
			CSVTHROW( "Rule " << name << " has invalid date sin range: "
							<< params[1] );
		}

		mMin = ALib::Date( tmp[0] );
		mMax = ALib::Date( tmp[1] );

		if ( mMin > mMax ) {
			CSVTHROW( "Rule " << name << " has invalid range: " << params[1] );
		}
		mCheckRange = true;
	}
	else {
		mCheckRange = false;
	}
}

//---------------------------------------------------------------------------
// Scrap reader created in Calidate()
//---------------------------------------------------------------------------

DateRule :: ~DateRule() {
	delete mDateReader;
}

//---------------------------------------------------------------------------
// Lazily create reader and use it to validate dates
//---------------------------------------------------------------------------

ValidationResult DateRule :: Validate( const CSVRow & row,
									unsigned int idx ) const  {

	if ( mDateReader == 0 ) {
		mDateReader = new MaskedDateReader( mMask );
	}

	ALib::Date dt;
	if ( idx < row.size() ) {
		bool ok = mDateReader->Read( row[idx], dt );
		if ( ! ok ) {
			return ValidationResult( idx,
					"Invalid date " + ALib::SQuote( row[idx] ) );
		}
	}

	if ( mCheckRange && (dt < mMin || dt > mMax ) ) {
		return ValidationResult( idx, "Date " + ALib::SQuote( row[idx] )
									+ " is out of range" );
	}

	return  ValidationResult();
}

//------------------------------------------------------------------------

} // end namespace

// end


