//
// Created by Arwa Hassan on 15/11/2025.
//

#ifndef QM_DD1_PITABLE_H
#define QM_DD1_PITABLE_H

#include <set>
#include <unordered_set>
#include "Implicant.h"

using ProductTerm = std::set<int>;
using BooleanExpression = std::vector<ProductTerm>;


class PItable {
public:
    static std::vector<ProductTerm> solvePIMatrixAndMinimize(const std::vector<Implicant>& primes,const std::vector<int>& essential, const std::vector<int>& remainingMinterms);
// the following are helper functions to help simpligy the boolean expression we reached
    static BooleanExpression multiplyExpressions(const BooleanExpression& exp1, const BooleanExpression& exp2);
    static BooleanExpression simplifyExpression(const BooleanExpression& exp);
};


#endif //QM_DD1_PITABLE_H