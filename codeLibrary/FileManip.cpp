//
// Created by Arwa Hassan on 14/11/2025.
//

#include "FileManip.h"

#include "PItable.h"


//Function 1: parseTerms

 std::vector<int> FileManip::parseTerms(const std::string &line, char expectedPrefix) {
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

 string FileManip::toBinary(int num, int bits) {
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

 int FileManip::doQMmin() {
    string testName;
    cout << "Enter test case (we have 5 tests and they follow this pattern: test1): ";
    cin >> testName;
//get the test file
    string filePath = "../testcases/" + testName + ".txt";
    if (!__fs::filesystem::exists(filePath)) {
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
    vector<Implicant> initial = Implicant::buildInitialImplicants(n, minterms, dontCares);
    Implicant::printImplicants(initial);
    // Generate Prime Implicants
    vector<Implicant> primeImplicants = Implicant::generatePrimeImplicants(initial, n);

    // Build quick sets for display classification
    unordered_set<int> mintermSet(minterms.begin(), minterms.end());
    unordered_set<int> dontCareSet(dontCares.begin(), dontCares.end());

    Implicant::printPrimeImplicants(primeImplicants, mintermSet, dontCareSet);
    vector<int> essential = Implicant::findEssentialPIs(primeImplicants, minterms);
    Implicant::printEssentialPIs(primeImplicants, essential, mintermSet);
    vector<int> remain = Implicant::remainingMintermsAfterEPIs(primeImplicants, essential, minterms);
    cout << "Uncovered minterms after EPIs: {";
    for (size_t i=0;i<remain.size();++i) {
        cout << remain[i];
        if (i+1<remain.size()) cout << ",";
    }
    cout << "}\n";
     std::vector<ProductTerm> minimizedSolutions = PItable::solvePIMatrixAndMinimize(primeImplicants,essential,remain);
      printMinimizedFunction(primeImplicants, essential, minimizedSolutions, n);
    return 0;
}

// Inside FileManip.cpp, for the function FileManip::printMinimizedFunction:

void FileManip::printMinimizedFunction(
    const std::vector<Implicant>& primes,
    const std::vector<int>& essential,
    const std::vector<ProductTerm>& minimalSolutions, int nbVars
) {
    using namespace std;
    std::string selectedFunc = "";

    // case1 only epis
    if (minimalSolutions.empty() && !essential.empty()) {
        std::cout << endl << "Minimized Boolean Function (1 Solution, only EPIs):" << endl;

        for (int idx : essential) {
            selectedFunc += Implicant::patternToBoolean(primes[idx].pattern);
            selectedFunc += " + ";
        }
        // removing trailing +s
        if (selectedFunc.size() > 3) {
            selectedFunc.resize(selectedFunc.size() - 3);
        }
        std::cout << "  F = " << selectedFunc << endl;

    }

    // cas2 petricks method solutions
    else {
        std::vector<std::string> allSolutions;

        // Print and store all solutions
        std::cout << endl << "Minimized Boolean Function (" << minimalSolutions.size() << " Solution(s)):" << endl;
        for (size_t s = 0; s < minimalSolutions.size(); ++s) {
            std::string currentFunc = "";

            // 1. Add Essential PIs
            for (int idx : essential) {
                currentFunc += Implicant::patternToBoolean(primes[idx].pattern);
                currentFunc += " + ";
            }

            // 2. adding the selected non essential PIs
            for (int idx : minimalSolutions[s]) {
                currentFunc += Implicant::patternToBoolean(primes[idx].pattern);
                currentFunc += " + ";
            }

            // cleaning up trailing +s
            if (currentFunc.size() > 3) {
                currentFunc.resize(currentFunc.size() - 3);
            }

            allSolutions.push_back(currentFunc); // storing the generated string
            std::cout << "  Solution " << s + 1 << ": F = " << currentFunc << "\n";
        }


        cout << endl << "Would you like to convert to Verilog code? (Insert 1 for yes): ";
        int choice;
        cin >> choice;

        if (choice == 1) {

            cout << endl << "Which of the boolean functions would you like to implement? (Enter a number from 1 to " << allSolutions.size() << "): ";
            int solution_index;
            cin >> solution_index;

            // Validate choice
            if (solution_index >= 1 && solution_index <= (int)allSolutions.size()) {
                selectedFunc = allSolutions[solution_index - 1];
            } else {
                std::cerr << "\nInvalid choice. Skipping Verilog generation." << std::endl;
                return;
            }
        } else {
            return;
        }
    } // end of 2nd case

    if (!selectedFunc.empty()) {
        cout << endl << "Insert the name for the Verilog module: ";
        string moduleName;
        cin >> moduleName;
        VerilogConverter::generateVerilogModule(moduleName, selectedFunc, nbVars, moduleName+".txt");
    }
    return;
}