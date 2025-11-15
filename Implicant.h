//
// Created by Arwa Hassan on 14/11/2025.
//

#ifndef QM_DD1_IMPLICANT_H
#define QM_DD1_IMPLICANT_H
using namespace std;
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

#include "FileManip.h"

//nadine: I created a structure specifically for the implicants to cover all their data
class Implicant {
public:
    string pattern;
    vector<int> covered;
    bool isPureDontCare;
    bool combined = false;

    //functions:


    static vector<Implicant> buildInitialImplicants(int n, const vector<int>& minterms, const vector<int>& dontCares);

    static void printImplicants(const vector<Implicant>& imps);

    static int countOnes(const string& pattern);

    static bool canCombine(const string& a, const string& b, string& outPattern);

    static void mergeCoverage(vector<int>& target, const vector<int>& add);

    static vector<Implicant> generatePrimeImplicants(const vector<Implicant>& initial, int n);

    static void printPrimeImplicants(const vector<Implicant>& primes,const unordered_set<int>& mintermSet,const unordered_set<int>& dontCareSet);

    static vector<vector<int>> buildPICoversMinterm(const vector<Implicant>& primes,const unordered_set<int>& mintermSet);

    static unordered_map<int, vector<int>> buildMintermToPIs(const vector<vector<int>>& piCovers);

    static vector<int> findEssentialPIs(const vector<Implicant>& primes,const vector<int>& minterms);

    static void printEssentialPIs(const vector<Implicant>& primes, const vector<int>& essential,const unordered_set<int>& mintermSet);

    static vector<int> remainingMintermsAfterEPIs(const vector<Implicant>& primes,const vector<int>& essential, const vector<int>& minterms);

    static string patternToBoolean(const std::string& pattern);

};


#endif //QM_DD1_IMPLICANT_H

