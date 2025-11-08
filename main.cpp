#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <bitset>
#include <filesystem>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;


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
    string bin = bitset<64>(num).to_string(); // 64 bits max
    return bin.substr(64 - bits); //keep only last 'bits' characters
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

    return 0;
}
