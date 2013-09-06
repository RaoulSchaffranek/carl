/** 
 * @file:   Reductor.h
 * @author: Sebastian Junges
 *
 * @since July 11, 2013
 */

#pragma once

#include "Ideal.h"
#include "ReductorEntry.h"
#include "../util/Heap.h"

namespace carl
{

/**
 *  Class with the settings for the reduction algorithm.
 */
template<class Polynomial>
class ReductorConfiguration
{
public:

	typedef ReductorEntry<Polynomial> EntryType;
	typedef EntryType* Entry;
	typedef carl::CompareResult CompareResult;

	static CompareResult compare(Entry e1, Entry e2)
	{
		return Polynomial::OrderedBy::compare(e1->getLead(), e2->getLead());
	}

	static bool cmpLessThan(CompareResult res)
	{
		return res == CompareResult::LESS;
	}
	static const bool supportDeduplicationWhileOrdering = false;

	static bool cmpEqual(CompareResult res)
	{
		return res == CompareResult::EQUAL;
	}

	/**
	 * should only be called if the compare result was EQUAL
	 * eliminate duplicate leading monomials
	 * @param e1 upper entry
	 * @param e2 lower entry
	 * @return true if e1->lt is cancelled
	 */
	static bool deduplicate(Entry e1, Entry e2)
	{
		assert(Polynomial::Ordering::compare(e1->getLead(), e2->getLead()) == CompareResult::EQUAL);
		return e1->addCoefficient(e2->getLead().getCoeff());
	}

	static const bool fastIndex = true;
};

template<typename T>
struct hasOrigins
{
	static const bool valid = true;
};

template<>
struct hasOrigins<void*>
{
	static const bool valid = false;
};

template<typename InputPolynomial, typename PolynomialInIdeal, typename Origins, template <class> class Datastructure = carl::Heap, template <typename Polynomial> class Configuration = ReductorConfiguration>
class Reductor
{
	static_assert(std::is_base_of<InputPolynomial, PolynomialInIdeal>::value,
				  "Ideal and polynomial to be reduced are incompatible");
protected:
	typedef typename InputPolynomial::OrderedBy Order;
	typedef typename Configuration<InputPolynomial>::EntryType EntryType;
	typedef typename InputPolynomial::CoeffType Coeff;
public:

	Reductor(const Ideal<PolynomialInIdeal>& ideal, const InputPolynomial& f) :
	mIdeal(ideal), mDatastruct(Configuration<InputPolynomial>()), mReductionOccured(false)
	{
		insert(f, new Term<Coeff>(Coeff(1)));
	}

	Reductor(const Ideal<PolynomialInIdeal>& ideal, const Term<Coeff> f) :
	mIdeal(ideal), mDatastruct(Configuration<InputPolynomial>())
	{
		insert(f);
	}

	virtual ~Reductor()
	{
	}

	/**
	 * The basic reduce routine on a priority queue.
	 * @return 
	 */
	bool reduce()
	{
		while(!mDatastruct.empty())
		{
			typename Configuration<InputPolynomial>::Entry entry;
			std::shared_ptr<const Term < Coeff >> leadingTerm;
			// Find a leading term.
			do
			{
				// get actual leading term
				entry = mDatastruct.top();
				leadingTerm = entry->getLead();
				LOGMSG_TRACE("carl.gb.reductor", "Intermediate leading term: " << *leadingTerm);
				assert(!leadingTerm->isZero());
				// update the data structure.
				// only insert non-emty polynomials.
				if(!updateDatastruct(entry)) break;
				typename Configuration<InputPolynomial>::Entry newentry = mDatastruct.top();
				while(entry != newentry && Order::equal(leadingTerm, newentry->getLead()))
				{
					entry->addCoefficient(newentry->getLead()->coeff());
					if(!updateDatastruct(newentry)) break;

					newentry = mDatastruct.top();
				}
				//                mDatastruct.print();
			}
			while(leadingTerm->isZero() && !mDatastruct.empty());
			// Done finding leading term.
			LOGMSG_DEBUG("carl.gb.reductor", "Leading term: " << *leadingTerm);
			// We have found the leading term..
			if(leadingTerm->isZero())
			{
				assert(mDatastruct.empty());
				//then the datastructure is empty, we are done.
				return true;
			}

			//find a suitable reductor and the corresponding factor.
			DivisionLookupResult<PolynomialInIdeal> divres(mIdeal.getDivisor(*leadingTerm));
			// check if the reduction succeeded.
			if(divres.success())
			{

				mReductionOccured = true;
				if(hasOrigins<Origins>::valid)
				{
					//mOrigins.calculateUnion(divres.mDivisor->getOrigins());
				}
				if(divres.mDivisor->nrTerms() > 1)
				{
					insert(divres.mDivisor->tail(), divres.mFactor);
				}
			}
			else
			{
				LOGMSG_DEBUG("carl.gb.reductor", "Not reducible: " << *leadingTerm);
				mRemainder.push_back(leadingTerm);
				return false;
			}

		}
		return true;
	}

	/**
	 * Gets the flag which indicates that a reduction has occurred  (p -> p' with p' != p)
	 * @return the value of the flag
	 */
	bool reductionOccured()
	{
		return mReductionOccured;
	}

	/**
	 * Uses the ideal to reduce a polynomial as far as possible.
	 * @return 
	 */
	InputPolynomial fullReduce()
	{
		// TODO:
		// Do simple reductions first.
		while(!reduce())
		{
			// no operation.
		}
		// TODO check whether this is sorted.
		return InputPolynomial(mRemainder.begin(), mRemainder.end(), false, false);
	}
private:

	/**
	 * A small routine which updates the underlying data structure for the polynomial which is reduced.
	 * @param entry
	 * @return 
	 */
	inline bool updateDatastruct(EntryType* entry)
	{
		if(entry->getTail().isZero())
		{
			mDatastruct.pop();
			delete entry;
			if(mDatastruct.empty()) return false;
		}
		else
		{
			entry->removeLeadingTerm();
			mDatastruct.decreaseTop(entry);
		}
		return true;
	}

	void insert(const InputPolynomial& g, const Term<Coeff>* fact)
	{
		if(!g.isZero())
		{
			LOGMSG_TRACE("carl.gb.reductor", "Insert polynomial: " << g << " * " << *fact);
			mDatastruct.push(new EntryType(fact, g));
		}
	}

	void insert(const Term<Coeff>& g)
	{
		assert(g.getCoeff() != 0);
		mDatastruct.push(new EntryType(g));
	}



	const Ideal<PolynomialInIdeal>& mIdeal;
	Datastructure<Configuration<InputPolynomial> > mDatastruct;
	std::vector<std::shared_ptr<const Term<Coeff >> > mRemainder;
	bool mReductionOccured;
	//Origins mOrigins;
};


}