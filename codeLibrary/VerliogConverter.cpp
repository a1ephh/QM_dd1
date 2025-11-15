//
// Created by Arwa Hassan on 15/11/2025.
//
#include "VerliogConverter.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std; // for the endl at the end

// helper function to convert normal boolean into verilog syntax
std::string VerilogConverter::convertToVerilogSyntax(const std::string& booleanFunction) {
    std::string verilog = booleanFunction;

    // 1. NOT operator A' --> ~A
    size_t pos = 0;
    while ((pos = verilog.find("'", pos)) != std::string::npos) {
        // variable right before the '
        if (pos > 0) {
            char var = verilog[pos - 1];
            // Replace the variable and the ' with the Verilog NOT syntax
            verilog.replace(pos - 1, 2, "~" + std::string(1, var));
        }
    }

    // OR operator + --> |

    std::replace(verilog.begin(), verilog.end(), '+', '|');

    // 3. AND operator A B-> A & B
    // This is the most complex step: Verilog requires explicit '&' between terms.
    std::string result = "";
    for (size_t i = 0; i < verilog.length(); ++i) {
        char current = verilog[i];
        result += current;

        if (i + 1 < verilog.length()) {
            char next = verilog[i + 1];

            // the criteria we are checking for are:
            // 1. current carr is variable or )
            // 2. next char is a variable (
            // 3. no verliog operator between the two characters


            bool currentIsVarOrNot = (std::isalpha(current) || current == ')');
            bool nextIsVarOrNot = (std::isalpha(next) || next == '(' || next == '~');

            if (currentIsVarOrNot && nextIsVarOrNot) {
                result += " & ";
            } else if (currentIsVarOrNot && next == '~') { // case A~B
                 result += " & ";
            } else if (current == '~' && std::isalpha(next) && (i + 2 < verilog.length() && verilog[i+2] != '|')) {
                 // case ~AB
                 if (std::isalpha(verilog[i+2])) {
                    result += " & ";
                 }
            }
        }
    }

    // cleaning up and removing spaces
    std::string cleaned_result = "";
    for (char c : result) {
        if (c != ' ' && c != '\t') {
            cleaned_result += c;
        }
    }

    std::string final_verilog = "";
    for (size_t i = 0; i < cleaned_result.length(); ++i) {
        char current = cleaned_result[i];
        final_verilog += current;

        if (std::isalpha(current) && i + 1 < cleaned_result.length() && std::isalpha(cleaned_result[i+1])) {
             final_verilog += "&";
        }
    }

    //replacingggg

    std::string minimal_verilog = booleanFunction;

    // Replace ' with ~
    for (size_t i = 0; i < minimal_verilog.length(); ++i) {
        if (minimal_verilog[i] == '\'') {
            if (i > 0) {
                minimal_verilog.replace(i - 1, 2, "~" + std::string(1, minimal_verilog[i-1]));
                i--; // Adjust index due to length change
            }
        }
    }


    std::replace(minimal_verilog.begin(), minimal_verilog.end(), '+', '|');


    std::string and_inserted = "";
    for (size_t i = 0; i < minimal_verilog.length(); ++i) {
        char current = minimal_verilog[i];
        and_inserted += current;

        if (i + 1 < minimal_verilog.length()) {
            char next = minimal_verilog[i + 1];

            // checking for adjacent variables
            bool is_variable_char = std::isalpha(current) || current == '~';
            bool next_is_variable_char = std::isalpha(next) || next == '~';

            // adding & if adjacent and not spereated by |
            if (is_variable_char && next_is_variable_char && current != '~' && next != '|') {
                 and_inserted += "&";
            }
        }
    }


    return and_inserted;
}

// Function that generates the final Verilog module
bool VerilogConverter::generateVerilogModule(
    const std::string& functionName,
    const std::string& booleanFunction,
    int numVariables,
    const std::string& outputFileName
) {
    if (booleanFunction.empty()) {
        std::cerr << "Error: Boolean function is empty.\n";
        return false;
    }
    std::string desiredOutputPath = "../verilogGenerationSamples/"+outputFileName;
    std::ofstream outFile(desiredOutputPath);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << outputFileName << " for writing.\n";
        return false;
    }

    // Determine input variables (A, B, C, D...)
    std::string inputs = "";
    for (int i = 0; i < numVariables; ++i) {
        inputs += (char)('A' + i);
        if (i < numVariables - 1) {
            inputs += ", ";
        }
    }

    // 1. Convert Boolean string to Verilog assign syntax
    std::string verilogAssign = convertToVerilogSyntax(booleanFunction);

    // 2. Construct the Verilog module
    outFile << "`timescale 1ns / 1ps" << endl;
    outFile << "//////////////////////////////////////////////////////////////////////////////////" << endl;
    outFile << "// Module: " << functionName << endl;
    outFile << "// Minimized Function (SOP): " << booleanFunction << endl;
    outFile << "//////////////////////////////////////////////////////////////////////////////////"<<endl;

    // Module Declaration
    outFile << "module " << functionName << "(" << endl;
    outFile << "    output wire F,"<<endl;
    outFile << "    input wire " << inputs << endl;
    outFile << ");" << endl << endl;

    outFile << "assign F = " << verilogAssign << ";\n\n";

    // End Module
    outFile << "endmodule\n";

    outFile.close();

    std::cout << endl << "Verilog module '" << functionName
              << "' has been successfully generated and and saved to " << outputFileName << endl;

    return true;
}