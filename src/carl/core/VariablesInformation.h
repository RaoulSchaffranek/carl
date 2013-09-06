/** 
 * @file:   VariablesInformation.h
 * @author: Sebastian Junges
 *
 * @since September 3, 2013
 */

#pragma once

#include "Variable.h"
#include "VarExpPair.h"
#include "VariableInformation.h"


namespace carl
{

template<bool collectCoeff, typename CoeffType>
class VariablesInformation
{
	std::map<Variable, VariableInformation<collectCoeff, CoeffType>> mVariableInfo;
	
	public:
	
	/**
	 * Updates the Variable informations based on the assumption that this method is called with according parameters
     * @param ve A variable-exponent pair occuring in a term t.
     * @param termCoeff The coefficient of t.
	 * @param monomial The monomial part of t.
     */
	template<typename TermCoeff>
	void variableInTerm(const VarExpPair& ve, const TermCoeff& termCoeff, const typename CoeffType::MonomType& monomial)
	{
		auto it = mVariableInfo.find(ve.var);
		// Variable did not occur before.
		if(it == mVariableInfo.end())
		{
			it = mVariableInfo.emplace(ve.var, VariableInformation<collectCoeff, CoeffType>(ve.exp)).first;
		}
		else
		{
			// One more term in which the variable occurs.
			(it->second.occurence)++;
			// Update minimal/maximal occurence.
			if(it->second.maxDegree < ve.exp)
			{
				it->second.maxDegree = ve.exp;
			}
			else if(it->second.minDegree > ve.exp) 
			{
				it->second.minDegree = ve.exp;
			}
		}
		if(collectCoeff)
		{
			typename CoeffType::MonomType* m = monomial.dropVariable(ve.var);
			if(m == nullptr)
			{
				it->second.updateCoeff(ve.exp, typename CoeffType::TermType(termCoeff));
			}
			else
			{
				it->second.updateCoeff(ve.exp, typename CoeffType::TermType(termCoeff, m));
			}
		}
	}
	
	const VariableInformation<collectCoeff, CoeffType>* getVarInfo(Variable::Arg v) const
	{
		auto it = mVariableInfo.find(v);
		// Variable did not occur before.
		if(it == mVariableInfo.end())
		{
			return nullptr;
		}
		else
		{
			return &(it->second);
		}
	}
	
	bool occurs(Variable::Arg v) const
	{
		return mVariableInfo.count(v) > 0;
	}
};
}