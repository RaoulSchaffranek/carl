#include "gtest/gtest.h"
#include "carl/core/TaylorExpansion.h"
#include <cln/cln.h>

using namespace carl;

TEST(TaylorExpansion, ideal_adic_coefficient) {
    
    // the Field Z_5
    GaloisFieldManager<cln::cl_I>& gfm = GaloisFieldManager<cln::cl_I>::getInstance();
    const GaloisField<cln::cl_I>* gf5 = gfm.getField(5,1);
    
    // the five numbers from Z_5
    GFNumber<cln::cl_I> a0(0, gf5);
    GFNumber<cln::cl_I> a1(1, gf5);
    GFNumber<cln::cl_I> a2(2, gf5);
    GFNumber<cln::cl_I> a3(3, gf5);
    GFNumber<cln::cl_I> a4(4, gf5);
    
    VariablePool& vpool = VariablePool::getInstance();
    Variable x_1 = vpool.getFreshVariable("x_1");
    Variable x_2 = vpool.getFreshVariable("x_2");
    
    MultivariatePolynomial<GFNumber<cln::cl_I>> p({a1 * x_1, a2 * x_2 * x_2, a1 * x_1 * x_2});
    
    // 
    std::cout << ideal_adic_coeff(p, x_2, a2, 2);
    
}
