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
#include <unordered_map>

#include "Implicant.h"

using namespace std;
// class for any file manipulations such as parsing
class FileManip {
public:
    static std::vector<int> parseTerms(const std::string &line, char expectedPrefix) ;

    static string toBinary(int num, int bits);
    static int doQMmin();
};


#endif //QM_DD1_FILEMANIP_H