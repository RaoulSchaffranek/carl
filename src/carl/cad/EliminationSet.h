/* 
 * @file   EliminationSet.h
 * @author Gereon Kremer <gereon.kremer@cs.rwth-aachen.de>
 */

#pragma once

#include <forward_list>
#include <list>
#include <set>
#include <utility>
#include <unordered_map>
#include <unordered_set>

#include "../core/UnivariatePolynomial.h"
#include "../core/logging.h"

#include "CADSettings.h"

namespace carl {
namespace CAD {
	
/**
 * 
 */
	
/** Reminder
 * - SingleEliminationQueue p -> PairedEliminationQueue mit p und p'
 * - UnivariatePolynomialPtr sind Multivariat!
 * - Ein EliminiationSet pro Variable
 */
template<typename Coefficient>
class EliminationSet {
	
// private types
private:
	
	/**
	 * Represents a pair of polynomials. 
	 * Used to store the ancestors of a polynomial. If one is nullptr, the polynomial has only a single ancestor.
	 */
	typedef std::pair<UnivariatePolynomialPtr<Coefficient>, UnivariatePolynomialPtr<Coefficient>> PolynomialPair;
	struct PolynomialPairIsLess {
		unsigned int length(const PolynomialPair& p) {
			if (p.first == nullptr && p.second == nullptr) return 0;
			if (p.first == nullptr || p.second == nullptr) return 1;
			return 2;
		}
		bool operator()(const PolynomialPair& a, const PolynomialPair& b) {
			unsigned int l1 = this->length(a), l2 = this->length(b);
			if (l1 < l2) return true;
			if (l2 < l1) return false;
			if (less(a.first, b.first)) return true;
			if (a.first == b.first) return less(a.second, b.second);
			return false;
		}
	};
	
	/**
	 * A set of polynomials.
	 */
	typedef std::unordered_set<UnivariatePolynomialPtr<Coefficient>> PolynomialSet;

	/**
	 * A mapping from one polynomial to a sorted range of other polynomials.
	 */
	typedef std::unordered_map<UnivariatePolynomialPtr<Coefficient>, PolynomialSet> PolynomialBucketMap;
	
	/// set of elimination parents
	typedef std::set<PolynomialPair, PolynomialPairIsLess> parentbucket;
	/// mapping one polynomial pointer to a set of elimination parents
	typedef std::unordered_map<UnivariatePolynomialPtr<Coefficient>, parentbucket> parentbucket_map;

	
// public types
public:
	typedef bool (*PolynomialComparator)( const UnivariatePolynomialPtr<Coefficient>&, const UnivariatePolynomialPtr<Coefficient>& );

// private members
private:	
	/**
	 * Set of polynomials.
	 */
	PolynomialSet polynomials;
	
	/**
	 * PolynomialComparator that defines the order of the polynomials in the elimination queue.
	 */
	PolynomialComparator eliminationOrder;
	
	/**
	 * PolynomialComparator that defines the order of the polynomials in the lifting queue.
	 */
	PolynomialComparator liftingOrder;
	
	/**
	 * Elimination queue containing all polynomials not yet considered for non-paired elimination.
	 * Access permits reset of the queue, automatic update after insertion of new elements and a pop method.
	 */
	std::list<UnivariatePolynomialPtr<Coefficient>> mSingleEliminationQueue;
	/**
	 * Elimination queue containing all polynomials not yet considered for paired elimination.
	 * Access permits reset of the queue, automatic update after insertion of new elements and a pop method.
	 */
	std::list<UnivariatePolynomialPtr<Coefficient>> mPairedEliminationQueue;
	
	/**
	 * Lifting queue containing all polynomials not yet considered for lifting.
	 * Access permits reset of the queue, automatic update after insertion of new elements and a pop method.
	 */
	std::list<UnivariatePolynomialPtr<Coefficient>> mLiftingQueue;
	/**
	 * Lifting queue containing a reset state for the lifting queue, which is a copy of the original container contents, but can be set to a concrete state
	 */
	std::list<UnivariatePolynomialPtr<Coefficient>> mLiftingQueueReset;
	
	/// maps an entry from another EliminationSet (parent) to the entry of elimination polynomial belonging to the parent
	PolynomialBucketMap childrenPerParent;
	/// assigns to each elimination polynomial its parents
	parentbucket_map parentsPerChild;
	
// public members
public:
	
	
	/*
	 * This flag indicates whether this elimination set contains polynomials that are only valid within certain bounds.
	 */
	bool bounded;
	
	//TODO: constructor
	
	///////////////
	// SELECTORS //
	///////////////
	
	std::list<UnivariatePolynomialPtr<Coefficient>&> getParentsOf(const UnivariatePolynomialPtr<Coefficient>& p) const;

	/**
	 * Checks if the given elimination polynomial has non-trivial parents, i.e. if it has more than a single parent.
     * @param p Univariate polynomial
     * @return true, if the given polynomial has non-trivial parents.
     */
	bool hasParents(const UnivariatePolynomialPtr<Coefficient>& p) const;
	
	/*
	 * Set a new order for the elimination queue.
	 * @param order New order function.
	 */
	void setEliminationOrder( PolynomialComparator order );
	/*
	 * Set a new order for the lifting queue.
	 * @param order New order function.
	 */
	void setLiftingOrder( PolynomialComparator order );
	
	////////////////////
	// ACCESS METHODS //
	////////////////////
	
	/**
	 * Inserts an elimination polynomial with the specified parent into the set.
	 * @param r elimination polynomial
	 * @param parents parents of the elimination (optional, standard is (0) ), if more than 1 parent is given, the list is interpreted as concatenation of parent pairs, e.g. (a, 0, b, c) defines  the parents (a) and (b,c).
	 * @param avoidSingle If true, the polynomial added is not added to the single-elimination queue (default: false).
	 * @return a pair, with its member pair::first set to an iterator pointing to either the newly inserted element or to the element that already had its same key in the map. The pair::second element in the pair is set to true if a new element was inserted or false if an element with the same key existed.
	 * @complexity amortized: logarithmic in the number of polynomials stored, worst: linear
	 * @see std::set::insert
	 */
	std::pair<typename PolynomialSet::iterator, bool> insert(
			const UnivariatePolynomialPtr<Coefficient> r,
			const std::list<UnivariatePolynomialPtr<Coefficient>>& parents = std::list<UnivariatePolynomialPtr<Coefficient>>( 1, UnivariatePolynomialPtr<Coefficient>() ),
			bool avoidSingle = false
			);
	
	/**
	 * Insert all polynomials from first to last (excl. last), while all have the same parent.
	 * @param first
	 * @param last
	 * @param parents parents of the elimination (optional, standard is one 0 parent)
	 * @param avoidSingle If true, all polynomials added are not added to the single-elimination queue (default: false).
	 * @return the list of polynomials actually added to the set, which might be smaller than the input set
	 */
	template<class InputIterator>
	std::list<UnivariatePolynomialPtr<Coefficient>> insert(
			InputIterator first,
			InputIterator last,
			const std::list<UnivariatePolynomialPtr<Coefficient>>& parents = std::list<UnivariatePolynomialPtr<Coefficient>>( 1, UnivariatePolynomialPtr<Coefficient>() ),
			bool avoidSingle = false
			);
	
	/**
	 * Insert an object which is allocated newly and stored as its new pointer value.
	 * @param r
	 * @param parents parents of the elimination (optional, standard is one 0 parent)
	 * @param avoidSingle If true, all polynomials added are not added to the single-elimination queue (default: false).
	 * @return a pair, with its member pair::first set to an iterator pointing to either the newly inserted element or to the element that already had its same key in the map. The pair::second element in the pair is set to true if a new element was inserted or false if an element with the same key existed.
	 * @complexity logarithmic in the number of polynomials stored
	 * @see insert
	 */
	std::pair<typename PolynomialSet::iterator, bool> insert(
			const UnivariatePolynomial<Coefficient>& r,
			const std::list<UnivariatePolynomialPtr<Coefficient>>& parents = std::list<UnivariatePolynomialPtr<Coefficient>>( 1, UnivariatePolynomialPtr<Coefficient>()),
			bool avoidSingle = false
			);
	
	/**
	 * Insert the contents of the set s into this set.
	 * @param s
	 * @param avoidSingle If true, all polynomials added are not added to the single-elimination queue (default: false).
	 * @return the list of polynomials actually added, which might be smaller than the input set
	 */
	std::list<UnivariatePolynomialPtr<Coefficient>> insert(
			const EliminationSet<Coefficient>& s,
			bool avoidSingle = false
			);
	/**
	 * Pretend to insert the contents of the set s into this set, but in fact, remove all polynomials <b>existing</b> in this set from s.
	 * @param s
	 * @return true if a change to s was committed
	 */
	bool insertAmend(EliminationSet<Coefficient>& s);
	
	/**
	 * Removes a polynomial p completely from the set (and all internal auxiliary structures).
	 * @param p polynomial to be removed from the set
	 * @return 1 if p exited in the set, 0 otherwise
	 * @complexity linear in the number of elimination polynomials one level above the level represented by this elimination set, i.e. the parents
	 */
	size_t erase(const UnivariatePolynomialPtr<Coefficient>& p);
	
	/** Asserts that parent is removed.
	 * Removes all elimination polynomials from the set which have parent as only parent (lone polynomials) or as only other parent (divorce-suffering polynomials).
	 * A list of those deleted is returned.
	 * Elimination polynomials which have several parents among which parent is are not deleted, but their auxiliary data structures are updated.
	 * @param parent of the lone elimination polynomials to be removed
	 * @return list of elimination polynomials removed
	 * @complexity linear in the number of elimination polynomials which do belong to the given parent
	 */
	std::forward_list<UnivariatePolynomialPtr<Coefficient>> removeByParent(const UnivariatePolynomialPtr<Coefficient>& parent);

	/**
	 * Searches the set entry for the given polynomial p if exists, otherwise nullptr.
	 * @param p
	 * @return set entry for the given polynomial p if exists, otherwise nullptr
	 */
	UnivariatePolynomialPtr<Coefficient> find(const UnivariatePolynomial<Coefficient>& p);
	
	/**
	 * Swaps the contents (all attributes) of the two EliminationSets.
	 * @see std::set::swap
	 */
	friend void swap(EliminationSet<Coefficient>& lhs, EliminationSet<Coefficient>& rhs);

	/**
	 * Remove every data from this set.
	 */
	void clear();
	
	/////////////////////////////////
	// LIFTING POSITION MANAGEMENT //
	/////////////////////////////////
	
	/**
	 * Retrieve the next position for lifting.
	 * The lifting positions are stored in the order of the set of elimination polynomials, but can lack polynomials which were already popped.
	 *
	 * If the lifting queue is empty the behavior of this method is undefined.
	 * @return the smallest (w.r.t. set order) elimination polynomial not yet considered for lifting
	 * @complexity constant
	 */
	const UnivariatePolynomialPtr<Coefficient>& nextLiftingPosition() {
		return this->mLiftingQueue.front();
	}

	/**
	 * Pop the polynomial returned by nextLiftingPosition() from the lifting position queue.
	 * The lifting positions are stored in the order of the set of elimination polynomials, but can lack polynomials which were already popped.
	 *
	 * If the lifting queue is empty the behavior of this method is undefined.
	 * @complexity constant
	 */
	void popLiftingPosition() {
		this->mLiftingQueue.pop_front();
	}

	/**
	 * Gives true if all were popped already, false if a lifting position exists.
	 * @return true if all were popped already, false if a lifting position exists
	 */
	bool emptyLiftingQueue() const {
		return this->mLiftingQueue.empty();
	}

	/**
	 * Gives true if the lifting position queue contains all elimination polynomials, false otherwise.
	 * @return true if the lifting position queue contains all elimination polynomials, false otherwise
	 */
	bool fullLiftingQueue() const {
		return this->mLiftingQueue.size() == this->polynomials.size();
	}

	/**
	 * Re-build the lifting position queue from scratch using all polynomials in the set. The reset state is <b>not</b> changed.
	 * @complexity linear in the number of polynomials stored
	 */
	void resetLiftingPositionsFully();
	
	/**
	 * Re-build the lifting position queue just with the polynomials stored as reset state.
	 * @complexity linear in the number of polynomials stored
	 */
	void resetLiftingPositions() {
		this->mLiftingQueue = this->mLiftingQueueReset;
	}

	/**
	 * Defines the reset state for lifting positions as the current lifting positions queue and all polynomials inserted in the future.
	 */
	void setLiftingPositionsReset() {
		this->mLiftingQueueReset = this->mLiftingQueue;
	}
	
	/////////////////////////////////////
	// ELIMINATION POSITION MANAGEMENT //
	/////////////////////////////////////
	
	/**
	 * Return the next position in the single-elimination queue and remove it from the queue.
	 *
	 * If the single-elimination queue is empty the behavior of this method is undefined.
	 * @return the next position in the single-elimination queue
	 */
	const UnivariatePolynomialPtr<Coefficient>& popNextSingleEliminationPosition();

	/**
	 * Gives true if all single eliminations are done.
	 * @return true if all single eliminations are done
	 */
	bool emptySingleEliminationQueue() const {
	   return mSingleEliminationQueue.empty();
	}

	/**
	 * Gives true if all paired eliminations are done, false otherwise.
	 * @return true if all eliminations are done, false otherwise.
	 */
	bool emptyPairedEliminationQueue() const {
	   return mPairedEliminationQueue.empty();
	}
	
	/**
	 * Does the elimination of the polynomial p and stores the resulting polynomials into the specified
	 * destination set.
	 *
	 * The polynomial is erased from all queues.
	 * @param p
	 * @param destination
	 * @param variable the main variable of the destination elimination set
	 * @param setting
	 * @return list of polynomials added to destination
	 */
	std::list<UnivariatePolynomialPtr<Coefficient>> eliminateInto(
			const UnivariatePolynomialPtr<Coefficient>& p,
			EliminationSet<Coefficient>& destination,
			const Variable& variable,
			const CADSettings& setting
			);

};

}
}

#include "EliminationSet.tpp"