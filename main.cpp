#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <bitset>
#include <filesystem>
#include <algorithm>
#include <limits>
#include <unordered_set>
#include <unordered_map>
using namespace std;
namespace fs = std::filesystem;
//I created a structure specifically for the implicants to cover all their data
struct Implicant {
    string pattern;
    vector<int> covered;
    bool isPureDontCare;
    bool combined = false;
};
//Function 1: parseTerms

vector<int> parseTerms(const string &line, char expectedPrefix) {
    vector<int> nums;
    if (line.empty()) return nums;

    //We first find the first non-space character and compare it with the expected (line1-> number, line2->m, like3->d)
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == string::npos) return nums;
    if (tolower(line[first]) != expectedPrefix) {
        cerr << "Error: Line must start with '" << expectedPrefix << "'. Instead got: " << line << endl;
        exit(1);
    }

    string token;
    stringstream ss(line);
    while (getline(ss, token, ',')) {
        // we just remove whitespace
        token.erase(remove_if(token.begin(), token.end(),
                              [](unsigned char c){ return c=='\r' || isspace(c); }),
                    token.end());
        if (token.empty()) continue; // skip blank tokens from

        //per-token prefix
        if (isalpha(static_cast<unsigned char>(token[0]))) {
            if (tolower(token[0]) != expectedPrefix) {
                //error handling when we don't have the right refix
                cerr << "Invalid token prefix: " << token
                     << " (expected '" << expectedPrefix << "')\n";
                exit(1);
            }
            if (token.size() == 1) {

                continue;
            }
            token.erase(0,1); //strip prefix
        }

        // check that all digits
        if (token.empty() ||
            !all_of(token.begin(), token.end(),
                    [](unsigned char c){ return isdigit(c); })) {
            cerr << "Invalid number: " << token << endl;
            exit(1);
                    }

        nums.push_back(stoi(token));
    }

    return nums;
}


// Function 2: toBinary

string toBinary(int num, int bits) {
    if (bits < 1 || bits > 64) {
        cerr << "Error: bits must be in [1, 64], got " << bits << ".\n"; //our code only accepts upto 64 bits
        exit(1);
    }
    if (num < 0) {
        cerr << "Error: negative term " << num << " is invalid.\n"; //we dont accept negative terms
        exit(1);
    }


    uint64_t maxVal = (bits == 64) ? std::numeric_limits<uint64_t>::max()
                                   : ((1ULL << bits) - 1ULL);

    if (static_cast<uint64_t>(num) > maxVal) {
        cerr << "Error: value " << num << " does not fit in " //if the value is bigger than the number of bits specified at the top of the file
             << bits << " bits (allowed 0.." << maxVal << ").\n";  //shows the max value for the bits allowed
        exit(1);
    }

    string bin = bitset<64>(num).to_string(); // 64 bits max
    return bin.substr(64 - bits); //keep only last 'bits' characters
}
//Function 3: Initial Implicants
vector<Implicant> buildInitialImplicants(int n,
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
        imp.pattern = toBinary(m, n);
        imp.covered = { m };
        imp.isPureDontCare = false; // contains a minterm
        result.push_back(std::move(imp));
    }
    //add dont-cares
    for (int d : dontCares) {
        Implicant imp;
        imp.pattern = toBinary(d, n);
        imp.covered = { d };
        imp.isPureDontCare = true; // only dont care
        result.push_back(std::move(imp));
    }
    return result;
}


//Function 4: Print implicants

void printImplicants(const vector<Implicant>& imps) {
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
int countOnes(const string& pattern) {
    return (int)std::count(pattern.begin(), pattern.end(), '1');
}


// Function 6: attempt combine two patterns
// Rules:
//  - They must differ in exactly one position where both have 0/1.
//  - No combination if a differing position contains '-' in either pattern.
//  - Other positions must be identical (including '-' matches).

bool canCombine(const string& a, const string& b, string& outPattern) {
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

void mergeCoverage(vector<int>& target, const vector<int>& add) {
    // Insert then deduplicate
    target.insert(target.end(), add.begin(), add.end());
    sort(target.begin(), target.end());
    target.erase(unique(target.begin(), target.end()), target.end());
}


// Function 8: Generate Prime Implicants

// Returns all prime implicants derived from initial implicants.
vector<Implicant> generatePrimeImplicants(const vector<Implicant>& initial, int n) {
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


void printPrimeImplicants(const vector<Implicant>& primes,
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
vector<vector<int>> buildPICoversMinterm(const vector<Implicant>& primes,
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

// Function 11: Map each minterm -> list of PI indices that cover it
unordered_map<int, vector<int>> buildMintermToPIs(const vector<vector<int>>& piCovers) {
    unordered_map<int, vector<int>> m2p;
    for (int i=0;i<(int)piCovers.size();++i)
        for (int m : piCovers[i])
            m2p[m].push_back(i);
    return m2p;
}

//Dunction 12:  Find essential PIs: a PI is essential if it is the sole cover for some minterm
vector<int> findEssentialPIs(const vector<Implicant>& primes,
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
void printEssentialPIs(const vector<Implicant>& primes,
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
vector<int> remainingMintermsAfterEPIs(const vector<Implicant>& primes,
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
// the main (wow)
int main() {
    string testName;
    cout << "Enter test case (we have 5 tests and they follow this pattern: test1): ";
    cin >> testName;
//get the test file
    string filePath = "../" + testName + ".txt";
    if (!fs::exists(filePath)) {
        cerr << "Error: file not found: " << filePath << endl;
        return 1;
    }
//since we need to have 3 lines even if there are no dont cares
    ifstream fin(filePath);
    string line1, line2, line3;
    if (!getline(fin, line1) || !getline(fin, line2) || !getline(fin, line3)) {
        cerr << "Error: file must have exactly 3 lines.\n";
        return 1;
    }
//check the maximum number of variables or if there are no variables
    int n = stoi(line1);
    if (n < 1 || n > 20) {
        cerr << "Error: number of variables must be between 1 and 20.\n";
        return 1;
    }
//here we need to specify the prefix for each like to use the parseTerms function
    vector<int> minterms  = parseTerms(line2, 'm'); // or 'M' supported automatically
    vector<int> dontCares = parseTerms(line3, 'd');
//to check if the binary conversion was done right
    cout << "\nParsed successfully!\n";
    cout << "Variables: " << n << endl;
    cout << "Minterms: ";
    for (int m : minterms) cout << m << " ";
    cout << "\nDon't cares: ";
    for (int d : dontCares) cout << d << " ";
    cout << "\n";

    cout << "\nBinary representations:\n";
    for (int m : minterms)
        cout << "m" << m << " = " << toBinary(m, n) << endl; //that's what I was talking about n->number of variables
    for (int d : dontCares)
        cout << "d" << d << " = " << toBinary(d, n) << endl;
    vector<Implicant> initial = buildInitialImplicants(n, minterms, dontCares);
    printImplicants(initial);
    // Generate Prime Implicants
    vector<Implicant> primeImplicants = generatePrimeImplicants(initial, n);

    // Build quick sets for display classification
    unordered_set<int> mintermSet(minterms.begin(), minterms.end());
    unordered_set<int> dontCareSet(dontCares.begin(), dontCares.end());

    printPrimeImplicants(primeImplicants, mintermSet, dontCareSet);
    vector<int> essential = findEssentialPIs(primeImplicants, minterms);
    printEssentialPIs(primeImplicants, essential, mintermSet);
    vector<int> remain = remainingMintermsAfterEPIs(primeImplicants, essential, minterms);
    cout << "Uncovered minterms after EPIs: {";
    for (size_t i=0;i<remain.size();++i) {
        cout << remain[i];
        if (i+1<remain.size()) cout << ",";
    }
    cout << "}\n";
    return 0;
}
