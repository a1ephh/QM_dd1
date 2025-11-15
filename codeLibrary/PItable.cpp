//
// Created by Arwa Hassan on 15/11/2025.
//

#include "PItable.h"




std::vector<ProductTerm> PItable::solvePIMatrixAndMinimize(
    const std::vector<Implicant>& primes,
    const std::vector<int>& essential,
    const std::vector<int>& remainingMinterms
) {
    // removing any minterms that are already included in the EPIs
    std::unordered_set<int> essentialSet(essential.begin(), essential.end());
    std::vector<int> nonEssentialIndices;
    for (size_t i = 0; i < primes.size(); ++i) {
        if (essentialSet.find(i) == essentialSet.end()) {
            nonEssentialIndices.push_back(i);
        }
    }

    if (remainingMinterms.empty()) {
        return {}; // no more PIs needed
    }
    if (nonEssentialIndices.empty()) {
        std::cerr << "Error: Uncovered minterms remaining but no non-essential PIs to cover them!\n";
        return {};
    }

    // POS (minterms*sums)
    std::vector<BooleanExpression> sumTerms;

    for (int minterm : remainingMinterms) {
        BooleanExpression currentSum;
        for (int pi_index : nonEssentialIndices) {
            // checking if this non-essential PI covers the minterm
            if (std::find(primes[pi_index].covered.begin(), primes[pi_index].covered.end(), minterm) != primes[pi_index].covered.end()) {
                currentSum.push_back({pi_index}); // Add PI as a single-element ProductTerm
            }
        }
        if (!currentSum.empty()) {
            sumTerms.push_back(currentSum);
        } else {
            std::cerr << "Error: Minterm " << minterm << " is uncovered by non-essential PIs.\n";
            return {};
        }
    }

    // multipkying minterms sequentially
    if (sumTerms.empty()) {
        return {}; // ahould not happen if remainingMinterms is not empty
    }

    BooleanExpression finalExpression = sumTerms[0];

    for (size_t i = 1; i < sumTerms.size(); ++i) {
        finalExpression = multiplyExpressions(finalExpression, sumTerms[i]);
    }

    // 4. Find the minimal term(s) (fewest PIs)
    size_t minSize = 1000;
    for (const auto& term : finalExpression) {
        if (term.size() < minSize) {
            minSize = term.size();
        }
    }

    // collect all minminal solutions
    std::vector<ProductTerm> minimalSolutions;
    for (const auto& term : finalExpression) {
        if (term.size() == minSize) {
            minimalSolutions.push_back(term);
        }
    }

    return minimalSolutions;
}

BooleanExpression PItable::multiplyExpressions(const BooleanExpression& exp1,
                                     const BooleanExpression& exp2) {
    BooleanExpression result;

    for (const auto& term1 : exp1) {
        for (const auto& term2 : exp2) {
            ProductTerm new_term = term1;
            new_term.insert(term2.begin(), term2.end());
            result.push_back(new_term);
        }
    }

    // simplifying via absorption law
    return simplifyExpression(result);
}

BooleanExpression PItable::simplifyExpression(const BooleanExpression& exp) {
    BooleanExpression simplified_exp;

    // sorting by size to check shortest first so its faster
    std::vector<ProductTerm> sorted_exp = exp;
    std::sort(sorted_exp.begin(), sorted_exp.end(),
              [](const ProductTerm& a, const ProductTerm& b) {
                  return a.size() < b.size();
              });

    for (size_t i = 0; i < sorted_exp.size(); ++i) {
        bool is_absorbed = false;
        //check if sorted_exp[i] is absorbed by any preceding, shorter term
        for (size_t j = 0; j < i; ++j) {
            // includes checks if sorted_exp[j] is a subset of sorted_exp[i]
            if (std::includes(sorted_exp[i].begin(), sorted_exp[i].end(),
                              sorted_exp[j].begin(), sorted_exp[j].end())) {

                is_absorbed = true;
                break;
                              }
        }

        if (!is_absorbed) {
            simplified_exp.push_back(sorted_exp[i]);
        }
    }
    return simplified_exp;
}