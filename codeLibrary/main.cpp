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
#include "Implicant.h"

using namespace std;
namespace fs = std::__fs::filesystem;


// the main (wow)
int main() {
    int n=1;
    while (n==1) {
        FileManip::doQMmin();
        std::cout << endl << "Would you like to test another ? Input 1 for yes. ";
        std::cin >> n;
    }
cout << endl << "Terminating program.";
}
