//
// Created by Arwa Hassan on 15/11/2025.
//
#ifndef VERILOGCONVERTER_H
#define VERILOGCONVERTER_H

#include <string>
#include <vector>


class VerilogConverter { // converts final boolean into verliog code
public:

    static bool generateVerilogModule(
        const std::string& functionName,
        const std::string& booleanFunction,
        int numVariables,
        const std::string& outputFileName
    );
//helper function:
    static std::string convertToVerilogSyntax(const std::string& booleanFunction);
};

#endif // VERILOGCONVERTER_H