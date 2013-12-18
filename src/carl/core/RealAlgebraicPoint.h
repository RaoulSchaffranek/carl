/* 
 * File:   RealAlgebraicPoint.h
 * Author: Gereon Kremer <gereon.kremer@cs.rwth-aachen.de>
 */

#pragma once

#include <vector>

#include "RealAlgebraicNumber.h"

namespace carl {

template<typename Number>
class RealAlgebraicPoint {
private:
	std::vector<RealAlgebraicNumber<Number>*> numbers;

public:
	/**
	 * Creates an empty Point of dimension 0.
	 */
	RealAlgebraicPoint():
		numbers()
	{}

	/**
	 * Copy constructor.
	 * @param r
	 */
	RealAlgebraicPoint(const RealAlgebraicPoint& r):
		numbers(r.numbers)
	{}

	/**
	 * Creates a real algebraic point with the specified components.
	 * @param v pointers to real algebraic numbers
	 */
	RealAlgebraicPoint(const std::vector<RealAlgebraicNumber<Number>*>& v):
		numbers(v)
	{}

	/**
	 * Creates a real algebraic point with the specified components from a list.
	 * @param v pointers to real algebraic numbers
	 */
	RealAlgebraicPoint(const std::list<RealAlgebraicNumber<Number>*>& v):
		numbers(v.begin(), v.end())
	{}

	/** Gives the number of components of this point.
	 * @return the dimension of this point
	 */
	long unsigned dim() const {
		return this->numbers.size();
	}

	/**
	 * Conjoins a point with a real algebraic number and returns the conjoined point as new object.
	 * e.g.: a point of dimension n conjoined with
	 * a real algebraic number is a point of dimension
	 * n+1.
	 * @param r additional dimension given as real algebraic number
	 * @return real algebraic point with higher dimension
	 */
	RealAlgebraicPoint conjoin( const RealAlgebraicNumber<Number>*& r ) {
		RealAlgebraicPoint res = RealAlgebraicPoint(*this);
		res.numbers.push_back(r);
		return res;
	}
	
	const RealAlgebraicNumber<Number>* operator[](unsigned int index) const {
		return this->numbers[index];
	}
	
	friend std::ostream& operator<<(std::ostream& os, const RealAlgebraicPoint<Number>& r) {
		os << "(";
		for (unsigned i = 0; i < r.dim(); i++) {
			if (i > 0) os << ", ";
			os << r.numbers[i];
		}
		os << ")";
		return os;
	}
};

}