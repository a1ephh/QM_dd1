//
// Created by Arwa Hassan on 14/11/2025.
//

#ifndef QM_DD1_FILEMANIP_H
#define QM_DD1_FILEMANIP_H
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
#include <set>
#include <unordered_map>

#include "Implicant.h"
#include "VerliogConverter.h"

using ProductTerm = std::set<int>;
using BooleanExpression = std::vector<ProductTerm>;

using namespace std;
class Implicant; // to fix the forward declaration error
// class for any file manipulations such as parsing
class FileManip {
public:
    static std::vector<int> parseTerms(const std::string &line, char expectedPrefix) ;

    static string toBinary(int num, int bits);
    static int doQMmin();
    static void printMinimizedFunction(const std::vector<Implicant> &primes, const std::vector<int> &essential, const std::vector<ProductTerm> &
                                       minimalSolutions, int nbVars
    );
};


#endif //QM_DD1_FILEMANIP_H