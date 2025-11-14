//
// Created by Arwa Hassan on 14/11/2025.
//

#include "Implicant.h"


//Function 3: Initial Implicants
vector<Implicant> Implicant::buildInitialImplicants(int n,
                                         const vector<int>& minterms,
                                         const vector<int>& dontCares) {
    vector<Implicant> result;
    result.reserve(minterms.size() + dontCares.size());

    //a quick membership test
    vector<char> isDont((1 << n), 0);
    for (int d : dontCares) isDont[d] = 1;

    //add minterms
    for (int m : minterms) {
        Implicant imp;
        imp.pattern = FileManip::toBinary(m, n);
        imp.covered = { m };
        imp.isPureDontCare = false; // contains a minterm
        result.push_back(std::move(imp));
    }
    //add dont-cares
    for (int d : dontCares) {
        Implicant imp;
        imp.pattern = FileManip::toBinary(d, n);
        imp.covered = { d };
        imp.isPureDontCare = true; // only dont care
        result.push_back(std::move(imp));
    }
    return result;
}


//Function 4: Print implicants

void Implicant::printImplicants(const vector<Implicant>& imps) {
    cout << "\nInitial implicants (" << imps.size() << "):\n";
    for (size_t i=0;i<imps.size();++i) {
        cout << "  [" << i << "] pattern=" << imps[i].pattern
             << " covered={";
        for (size_t k=0;k<imps[i].covered.size();++k) {
            cout << imps[i].covered[k];
            if (k+1 < imps[i].covered.size()) cout << ",";
        }
        cout << "} pureDontCare=" << (imps[i].isPureDontCare ? "yes" : "no") << "\n";
    }
}

// Function 5: count '1's ignoring '-'
int Implicant::countOnes(const string& pattern) {
    return (int)std::count(pattern.begin(), pattern.end(), '1');
}


// Function 6: attempt combine two patterns
// Rules:
//  - They must differ in exactly one position where both have 0/1.
//  - No combination if a differing position contains '-' in either pattern.
//  - Other positions must be identical (including '-' matches).

bool Implicant::canCombine(const string& a, const string& b, string& outPattern) {
    if (a.size() != b.size()) return false;
    int diffCount = 0;
    size_t diffPos = 0;
    for (size_t i=0;i<a.size();++i) {
        char ca = a[i];
        char cb = b[i];
        if (ca == cb) continue;
        // If either is '-', cannot combine on that difference
        if (ca == '-' || cb == '-') return false;
        // Must be a 0/1 difference
        if ((ca == '0' && cb == '1') || (ca == '1' && cb == '0')) {
            diffCount++;
            diffPos = i;
            if (diffCount > 1) return false;
        } else {
            // Some unexpected character difference
            return false;
        }
    }
    if (diffCount == 1) {
        outPattern = a;
        outPattern[diffPos] = '-';
        return true;
    }
    return false;
}


//Function 7: Deduplicate & merge coverage

void Implicant::mergeCoverage(vector<int>& target, const vector<int>& add) {
    // Insert then deduplicate
    target.insert(target.end(), add.begin(), add.end());
    sort(target.begin(), target.end());
    target.erase(unique(target.begin(), target.end()), target.end());
}


// Function 8: Generate Prime Implicants

// Returns all prime implicants derived from initial implicants.
vector<Implicant> Implicant::generatePrimeImplicants(const vector<Implicant>& initial, int n) {
    vector<Implicant> current = initial;
    vector<Implicant> primes;

    // Loop passes until no new combinations
    while (!current.empty()) {
        // Group by count of ones
        int maxGroupIndex = n; // worst-case
        vector<vector<int>> groups(maxGroupIndex + 1); // store indices
        for (size_t i=0;i<current.size();++i) {
            int ones = countOnes(current[i].pattern);
            groups[ones].push_back((int)i);
        }

        // Map new pattern -> index in next vector
        unordered_map<string, int> newIndex;
        vector<Implicant> nextPass;

        // Reset combined flags for this pass
        for (auto& imp : current) {
            imp.combined = false;
        }

        // Combine adjacent groups
        for (int g=0; g<maxGroupIndex; ++g) {
            if (groups[g].empty() || groups[g+1].empty()) continue;
            for (int idxA : groups[g]) {
                for (int idxB : groups[g+1]) {
                    string newPattern;
                    if (canCombine(current[idxA].pattern, current[idxB].pattern, newPattern)) {
                        // Mark originals as combined
                        current[idxA].combined = true;
                        current[idxB].combined = true;

                        auto it = newIndex.find(newPattern);
                        if (it == newIndex.end()) {
                            Implicant newImp;
                            newImp.pattern = newPattern;
                            newImp.covered = current[idxA].covered;
                            mergeCoverage(newImp.covered, current[idxB].covered);
                            newImp.isPureDontCare = current[idxA].isPureDontCare && current[idxB].isPureDontCare;
                            newImp.combined = false;
                            int newPos = (int)nextPass.size();
                            nextPass.push_back(std::move(newImp));
                            newIndex[newPattern] = newPos;
                        } else {
                            // Merge coverage with existing implicant having same pattern
                            mergeCoverage(nextPass[it->second].covered, current[idxA].covered);
                            mergeCoverage(nextPass[it->second].covered, current[idxB].covered);
                            nextPass[it->second].isPureDontCare = nextPass[it->second].isPureDontCare &&
                                                                  current[idxA].isPureDontCare &&
                                                                  current[idxB].isPureDontCare;
                        }
                    }
                }
            }
        }

        // Any implicant not combined in this pass becomes a prime implicant
        for (auto& imp : current) {
            if (!imp.combined) {
                // Avoid duplicate prime implicants with same pattern & coverage:
                auto it = find_if(primes.begin(), primes.end(),
                                  [&](const Implicant& p){ return p.pattern == imp.pattern; });
                if (it == primes.end()) {
                    primes.push_back(imp);
                } else {
                    mergeCoverage(it->covered, imp.covered);
                    it->isPureDontCare = it->isPureDontCare && imp.isPureDontCare;
                }
            }
        }

        // Prepare for next pass
        current = std::move(nextPass);
    }

    return primes;
}


// Function 9: Printing helpers


void Implicant::printPrimeImplicants(const vector<Implicant>& primes,
                          const unordered_set<int>& mintermSet,
                          const unordered_set<int>& dontCareSet) {
    cout << "\nPrime Implicants (" << primes.size() << "):\n";
    for (size_t i=0;i<primes.size();++i) {
        const auto& imp = primes[i];
        // Separate coverage into minterms and dont-cares for clarity
        vector<int> mins;
        vector<int> dcs;
        for (int v : imp.covered) {
            if (mintermSet.count(v)) mins.push_back(v);
            else if (dontCareSet.count(v)) dcs.push_back(v);
        }
        sort(mins.begin(), mins.end());
        sort(dcs.begin(), dcs.end());

        cout << "  PI[" << i << "] pattern=" << imp.pattern << "  covers minterms={";
        for (size_t k=0;k<mins.size();++k) {
            cout << mins[k];
            if (k+1<mins.size()) cout << ",";
        }
        cout << "} dontCares={";
        for (size_t k=0;k<dcs.size();++k) {
            cout << dcs[k];
            if (k+1<dcs.size()) cout << ",";
        }
        cout << "}\n";
    }
}
// Function 10: Build, for each PI, the list of minterms (exclude don’t cares)
vector<vector<int>> Implicant::buildPICoversMinterm(const vector<Implicant>& primes,
                                         const unordered_set<int>& mintermSet) {
    vector<vector<int>> piCovers(primes.size());
    for (size_t i=0;i<primes.size();++i) {
        for (int v : primes[i].covered)
            if (mintermSet.count(v))
                piCovers[i].push_back(v);
        sort(piCovers[i].begin(), piCovers[i].end());
        piCovers[i].erase(unique(piCovers[i].begin(), piCovers[i].end()), piCovers[i].end());
    }
    return piCovers;
}

// Function 11: Implicant::Map each minterm -> list of PI indices that cover it
unordered_map<int, vector<int>> Implicant::buildMintermToPIs(const vector<vector<int>>& piCovers) {
    unordered_map<int, vector<int>> m2p;
    for (int i=0;i<(int)piCovers.size();++i)
        for (int m : piCovers[i])
            m2p[m].push_back(i);
    return m2p;
}

//Dunction 12:  Find essential PIs: a PI is essential if it is the sole cover for some minterm
vector<int> Implicant::findEssentialPIs(const vector<Implicant>& primes,
                             const vector<int>& minterms) {
    unordered_set<int> mintermSet(minterms.begin(), minterms.end());
    auto piCovers = buildPICoversMinterm(primes, mintermSet);
    auto m2p = buildMintermToPIs(piCovers);

    vector<int> essential;
    for (int m : minterms) {
        auto it = m2p.find(m);
        if (it == m2p.end() || it->second.empty()) {
            cerr << "Error: no PI covers minterm " << m << "\n";
            continue;
        }
        if (it->second.size() == 1)
            essential.push_back(it->second.front());
    }
    sort(essential.begin(), essential.end());
    essential.erase(unique(essential.begin(), essential.end()), essential.end());
    return essential;
}
//Function 13: Printing helper
void Implicant::printEssentialPIs(const vector<Implicant>& primes,
                       const vector<int>& essential,
                       const unordered_set<int>& mintermSet) {
    cout << "\nEssential Prime Implicants (" << essential.size() << "):\n";
    for (int idx : essential) {
        vector<int> mins;
        for (int v : primes[idx].covered)
            if (mintermSet.count(v))
                mins.push_back(v);
        sort(mins.begin(), mins.end());
        cout << "  EPI[" << idx << "] pattern=" << primes[idx].pattern << " covers minterms={";
        for (size_t k=0;k<mins.size();++k) {
            cout << mins[k];
            if (k+1<mins.size()) cout << ",";
        }
        cout << "}\n";
    }
}

//Function 14: Compute minterms still uncovered after taking EPIs
vector<int> Implicant::remainingMintermsAfterEPIs(const vector<Implicant>& primes,
                                       const vector<int>& essential,
                                       const vector<int>& minterms) {
    unordered_set<int> covered;
    for (int idx : essential)
        for (int v : primes[idx].covered)
            covered.insert(v);
    vector<int> remain;
    for (int m : minterms)
        if (!covered.count(m))
            remain.push_back(m);
    return remain;
}