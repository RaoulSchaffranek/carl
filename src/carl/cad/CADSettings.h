/* 
 * File:   CADSettings.h
 * Author: Gereon Kremer <gereon.kremer@cs.rwth-aachen.de>
 */

#pragma once

namespace carl {
namespace CAD {

/** Predefined settings for the CAD procedure.
 * Implementation of the types is located in CAD.h. Each setting is defined as a power of two in order to use several flags at a time.
 * Note that the order of the flags plays a role: If, e.g., A_CADSETTING and B_CADSETTING are set, then the last one (B_CADSETTING) is used.
 */
enum CADSettingsType {
   /// generic setting: low-degree first polynomial order, nothing more
   GENERIC_CADSETTING = 1,
   /// setting avoiding computations with interval-represented samples (low- and odd-degree order + preference of numeric samples)
   RATIONALSAMPLE_CADSETTING = 2,
   /// setting preferring computations with interval-represented samples (low- and even-degree order + preference of root samples)
   IRRATIONALSAMPLE_CADSETTING = 4,
   /// setting for an equation-only input to CAD::check
   EQUATIONSONLY_CADSETTING = 8,
   /// setting for an inequality-only input to CAD::check
   INEQUALITIESONLY_CADSETTING = 16,
   /// equation- and inequality parts are treated separately, the equation part is solved first and checked against the other constraints
   EQUATIONDETECT_CADSETTING = 32,
   /// like EQUATIONDETECT_CADSETTING, but the equation part is assumed to be zero-dimensional
   ZERODIM_CADSETTING = 64,
   /// alternative polynomial ordering
   ALTERNATIVEORDER_CADSETTING = 128,
   /// everything optimized for the use with bounds
   BOUNDED_CADSETTING = 256,
   /// bounds-related optimizations explicitely deactivated
   NOTBOUNDED_CADSETTING = 512
};

/// The default setting for CAD settings, which is chosen if the CAD object is initialized without any other parameter.
static const unsigned DEFAULT_CADSETTING = BOUNDED_CADSETTING;

struct CADSettings {
public:
	/// flag indicating that the construction of new samples (by taking a new polynomial for lifting) is preferred to the choice of IR samples (PreferNRSamples excludes the use of PreferSamplesByIsRoot and vice versa)
	bool preferNRSamples;
	/// flag indicating that the choice of samples is guided by either being a root or not (PreferSamplesByIsRoot excludes the use of PreferNRSamples and vice versa)
	bool preferSamplesByIsRoot;
	/// flag indicating that the construction of new samples (by taking a new polynomial for lifting) is preferred to take root samples, if and only if mPreferSamplesByIsRoot is on
	bool preferNonrootSamples;
	/// flag indicating that Groebner bases are used to simplify the input polynomials corresponding to equations
	bool simplifyByGroebner;
	/// flag indicating that the elimination uses real root counting to simplify the bottom-most level
	bool simplifyByRootcounting;
	/// flag indicating that the elimination uses factorization of polynomials in every level
	bool simplifyByFactorization;
	/// flag activating the optimizations of [McCallum - "An Improved Projection Operation for Cylindrical Algebraic Decomposition of Three-dimensional Space"]
	bool simplifyFor3D;
	/// elimination following [McCallum - "On projection in CAD-based quantifier elimination with equational constraint"] and [McCallum - "On Propagation of Equational Constraints in CAD-Based Quantifier Elimination"], and no intermediate points are considered for lifting
	bool equationsOnly;
	/// only intermediate points are considered for lifting [McCallum - "Solving Polynomial Strict Inequalities Using Cylindrical Algebraic Decomposition"]
	bool inequalitiesOnly;
	/// during one elimination operation, all polynomials which are just copied to the next level are removed from the previous
	bool removeConstants;
	/// if a polynomial is removed from the CAD, remove also those variables and the respective elimination and sample levels which correspond to empty elimination levels
	bool trimVariables;
	/// treat equations separately by tuning the cad object to equations
	bool autoSeparateEquations;
	/// if equationsOnly is set, tune the CAD as if all input equation systems are zero-dimensional
	bool zeroDimEquations;
	/// compute a conflict graph after determining unsatisfiability of a set of constraints via CAD::check
	bool computeConflictGraph;
	/// number of points to be used for the premise of the deduction computed after determining satisfiability of a set of constraints via CAD::check (0: give constraints for the whole CAD cell so that the resulting implication can be quantified universally, k>0: take at most k points for existential quantification)
	unsigned numberOfDeductions;
	/// use the precomputed and maintained sample trace mTrace for the check of a new set of constraints whenever possible
	bool warmRestart;
	/// given bounds to the check method, these bounds are used to solve the constraints just by interval arithmetic
	bool preSolveByBounds;
	/// given bounds to the check method, these bounds are used wherever possible to reduce the sample sets during the lifting and to reduce the elimination polynomials if simplifyEliminationByBounds is set
	bool earlyLiftingPruningByBounds;
	/// given bounds to the check method, these bounds are used to cancel out elimination polynomials
	bool simplifyEliminationByBounds;
	/// given bounds to the check method, the bounds are widened after determining unsatisfiability by check, or shrunk after determining satisfiability by check
	bool improveBounds;
	/// the order in which the polynomials in each elimination level are sorted
	//bool (*up_isLess)( const UnivariatePolynomialPtr&, const UnivariatePolynomialPtr& );
	/// standard strategy to be used for real root isolation
	//RealAlgebraicNumberSettings::IsolationStrategy isolationStrategy;

	/**
	 * Generate a CADSettings instance of the respective preset type.
	 * @param setting preset
	 * @param isolationStrategy strategy for isolating real roots
	 * @param cadSettings setting to be enhanced (default: CADSettings())
	 * @return a CADSettings instance of the respective preset type
	 */
	static CADSettings getSettings(
			unsigned setting = DEFAULT_CADSETTING,
			//RealAlgebraicNumberSettings::IsolationStrategy isolationStrategy = RealAlgebraicNumberSettings::DEFAULT_ISOLATIONSTRATEGY,
			CADSettings cadSettings = CADSettings() )
	{
		//cadSettings.isolationStrategy = isolationStrategy;
		if (setting & RATIONALSAMPLE_CADSETTING) {
			cadSettings.autoSeparateEquations = false;
			cadSettings.preferNRSamples       = true;
			//cadSettings.up_isLess             = UnivariatePolynomial::univariatePolynomialIsLessOddCB;
		}
		if (setting & IRRATIONALSAMPLE_CADSETTING) {
			cadSettings.autoSeparateEquations = false;
			cadSettings.preferNRSamples       = false;
			//cadSettings.up_isLess             = UnivariatePolynomial::univariatePolynomialIsLessEvenCB;
		}
		if (setting & EQUATIONDETECT_CADSETTING) {
			cadSettings.autoSeparateEquations = true;
			//cadSettings.up_isLess             = UnivariatePolynomial::univariatePolynomialIsLessCB;
		}
		if (setting & BOUNDED_CADSETTING) {
			cadSettings.autoSeparateEquations       = true;
			cadSettings.computeConflictGraph        = false;
			cadSettings.numberOfDeductions          = 0;
			cadSettings.earlyLiftingPruningByBounds = true;
			cadSettings.improveBounds               = true;
			cadSettings.preSolveByBounds            = false;
			cadSettings.removeConstants             = true;
			cadSettings.simplifyByFactorization     = true;
			cadSettings.simplifyByRootcounting      = false;
			cadSettings.simplifyEliminationByBounds = true;
			cadSettings.trimVariables               = false;
			cadSettings.warmRestart                 = true;
			//cadSettings.up_isLess                   = UnivariatePolynomial::univariatePolynomialIsLessCB;
		}
		if (setting & NOTBOUNDED_CADSETTING) {
			cadSettings.autoSeparateEquations       = true;
			cadSettings.computeConflictGraph        = false;
			cadSettings.numberOfDeductions          = 0;
			cadSettings.earlyLiftingPruningByBounds = false;
			cadSettings.improveBounds               = false;
			cadSettings.preSolveByBounds            = false;
			cadSettings.removeConstants             = true;
			cadSettings.simplifyByFactorization     = true;
			cadSettings.simplifyByRootcounting      = false;
			cadSettings.simplifyEliminationByBounds = false;
			cadSettings.trimVariables               = false;
			cadSettings.warmRestart                 = true;
			//cadSettings.up_isLess                   = UnivariatePolynomial::univariatePolynomialIsLessCB;
		}
		if (setting & EQUATIONSONLY_CADSETTING) {
			cadSettings.autoSeparateEquations = false;
			cadSettings.preferNRSamples       = false;
			cadSettings.equationsOnly         = true;
			cadSettings.inequalitiesOnly      = false;
			cadSettings.preferSamplesByIsRoot = true;
			cadSettings.preferNonrootSamples  = false;    // this has only effect if there are still samples in the cad which belong to inequalities
			//cadSettings.up_isLess = UnivariatePolynomial::univariatePolynomialIsLessCB;
		}
		if (setting & INEQUALITIESONLY_CADSETTING) {
			cadSettings.preferNRSamples       = false;
			cadSettings.equationsOnly         = false;
			cadSettings.inequalitiesOnly      = true;
			cadSettings.preferSamplesByIsRoot = true;
			cadSettings.preferNonrootSamples  = true;    // this has only effect if there are still samples in the cad which belong to equations
			//cadSettings.up_isLess             = UnivariatePolynomial::univariatePolynomialIsLessCB;
		}
		if (setting & ZERODIM_CADSETTING) {
			cadSettings.zeroDimEquations = true;
		}
		if (setting & ALTERNATIVEORDER_CADSETTING) {
			//cadSettings.up_isLess = UnivariatePolynomial::univariatePolynomialIsLessLowDeg;
		}

		return cadSettings;
	}

	friend std::ostream& operator<<(std::ostream& os, const CADSettings& settings) {
		std::list<std::string> settingStrs;
		if (settings.simplifyByGroebner)
			settingStrs.push_back( "Simplify the input polynomials corresponding to equations by a Groebner basis (currently disabled)." );
		if (settings.simplifyByRootcounting)
			settingStrs.push_back( "Simplify the base elimination level by real root counting." );
		if (settings.simplifyByFactorization)
			settingStrs.push_back( "Simplify the elimination by factorization of polynomials in every level (using GiNaC::factor)." );
		if (settings.simplifyFor3D)
			settingStrs.push_back( "Simplify the elimination of trivariate polynomials (currently disabled)." );
		if (settings.preferNRSamples)
			settingStrs.push_back( "Prefer numerics to interval representations for sample choice." );
		if (settings.preferSamplesByIsRoot && settings.preferNonrootSamples)
			settingStrs.push_back( "Prefer non-root to root samples for sample choice." );
		if (settings.preferSamplesByIsRoot &&!settings.preferNonrootSamples)
			settingStrs.push_back( "Prefer root to non-root samples for sample choice." );
		if (settings.equationsOnly)
			settingStrs.push_back( "Simplify elimination for equation-only use (currently disabled) + do not use intermediate points for lifting." );
		if (settings.inequalitiesOnly)
			settingStrs.push_back( "Use only intermediate points for lifting." );
		if (settings.removeConstants)
			settingStrs.push_back( "During elimination, all polynomials which are just copied to the next level are removed from the previous." );
		if (settings.trimVariables)
			settingStrs.push_back( "If a polynomial is removed from the CAD, remove also those variables and the respective elimination and sample levels which correspond to empty elimination levels." );
		if (settings.autoSeparateEquations)
			settingStrs.push_back( "Treat equations separately by tuning the cad object to equations." );
		if (settings.zeroDimEquations)
			settingStrs.push_back( "If equationsOnly is set, tune the CAD as if all input equation systems are zero-dimensional." );
		if (settings.computeConflictGraph)
			settingStrs.push_back( "Compute a conflict graph after determining unsatisfiability of a set of constraints via CAD::check." );
		if (settings.warmRestart)
			settingStrs.push_back( "Use the precomputed and maintained sample trace CAD::mTrace for the check of a new set of constraints whenever possible." );
		if (settings.preSolveByBounds)
			settingStrs.push_back( "Given bounds to the check method, these bounds are used to solve the constraints just by interval arithmetic." );
		if (settings.earlyLiftingPruningByBounds)
			settingStrs.push_back( "Given bounds to the check method, these bounds are used to reduce the sample sets during the lifting and to reduce the elimination polynomials if simplifyEliminationByBounds is set." );
		if (settings.simplifyEliminationByBounds)
			settingStrs.push_back( "Given bounds to the check method, these bounds are used to cancel out elimination polynomials." );
		if (settings.improveBounds)
			settingStrs.push_back( "Given bounds to the check method, the bounds are widened after determining unsatisfiability by check, or shrunk after determining satisfiability by check." );
		std::string orderStr = "Polynomial order: ";
		/*
		if (settings.up_isLess == UnivariatePolynomial::univariatePolynomialIsLess)
			settingStrs.push_back( orderStr + "Default ordering relying on the term order implemented in GiNaC." );
		if (settings.up_isLess == UnivariatePolynomial::univariatePolynomialIsLessCB)
			settingStrs.push_back( orderStr + "Take polynomials with small roots first." );
		if (settings.up_isLess == UnivariatePolynomial::univariatePolynomialIsLessLowDeg)
			settingStrs.push_back( orderStr + "Take polynomials with few roots first.");
		if (settings.up_isLess == UnivariatePolynomial::univariatePolynomialIsLessOddLowDeg)
			settingStrs.push_back( orderStr + "Take polynomials with rational roots first, then with few roots." );
		if (settings.up_isLess == UnivariatePolynomial::univariatePolynomialIsLessOddCB)
			settingStrs.push_back( orderStr + "Take polynomials with rational roots first, then small roots." );
		if (settings.up_isLess == UnivariatePolynomial::univariatePolynomialIsLessEvenLowDeg)
			settingStrs.push_back( orderStr + "Take polynomials with irrational roots first, then few roots." );
		if (settings.up_isLess == UnivariatePolynomial::univariatePolynomialIsLessEvenCB)
			settingStrs.push_back( orderStr + "Take polynomials with irrational roots first, then small roots." );
		if (settings.up_isLess == UnivariatePolynomial::univariatePolynomialIsLessActivity)
			settingStrs.push_back( orderStr + "Take polynomials with lower activity first." );
		*/
		os << "+------------------------------------ CAD Setting -----------------------------------";
		if (settingStrs.empty()) {
			os << std::endl << "| Default ";
		} else {
			for (auto i: settingStrs) {
				os << std::endl << "↳ " << i;
			}
		}
		return os << std::endl << "+------------------------------------------------------------------------------------";
	}

private:

	/**
	 * Constructor initiating a standard settings object.
	 */
	CADSettings():
		preferNRSamples( false ),
		preferSamplesByIsRoot( false ),
		preferNonrootSamples( false ),
		simplifyByGroebner( false ),
		simplifyByRootcounting( false ),
		simplifyByFactorization( true ),
		simplifyFor3D( false ),
		equationsOnly( false ),
		inequalitiesOnly( false ),
		removeConstants( true ),
		trimVariables( false ),
		autoSeparateEquations( false ),
		zeroDimEquations( false ),
		computeConflictGraph( true ),
		numberOfDeductions( 1 ),
		warmRestart( false ),
		preSolveByBounds( false ),
		earlyLiftingPruningByBounds( true ),
		simplifyEliminationByBounds( true ),
		improveBounds( true )
		//up_isLess( UnivariatePolynomial::univariatePolynomialIsLess ),
		//isolationStrategy( RealAlgebraicNumberSettings::DEFAULT_ISOLATIONSTRATEGY )
	{}

public:

	/**
	 * Copy constructor.
	 */
	CADSettings( const CADSettings& s ):
		preferNRSamples( s.preferNRSamples ),
		preferSamplesByIsRoot( s.preferSamplesByIsRoot ),
		preferNonrootSamples( s.preferNonrootSamples ),
		simplifyByGroebner( s.simplifyByGroebner ),
		simplifyByRootcounting( s.simplifyByRootcounting ),
		simplifyByFactorization( s.simplifyByFactorization ),
		simplifyFor3D( s.simplifyFor3D ),
		equationsOnly( s.equationsOnly ),
		inequalitiesOnly( s.inequalitiesOnly ),
		removeConstants( s.removeConstants ),
		trimVariables( s.trimVariables ),
		autoSeparateEquations( s.autoSeparateEquations ),
		zeroDimEquations( s.zeroDimEquations ),
		computeConflictGraph( s.computeConflictGraph ),
		numberOfDeductions( s.numberOfDeductions ),
		warmRestart( s.warmRestart ),
		preSolveByBounds( s.preSolveByBounds ),
		earlyLiftingPruningByBounds( s.earlyLiftingPruningByBounds ),
		simplifyEliminationByBounds( s.simplifyEliminationByBounds ),
		improveBounds( s.improveBounds )
		//up_isLess( s.up_isLess ),
		//isolationStrategy( s.isolationStrategy )
	{}
};

}
}