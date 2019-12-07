//
// Created by Kacper Raczy on 2019-11-26.
//

#include <iostream>
#include <exception>
#include <list>
#include <unistd.h>

// frontend
extern "C" {
#define operator op
#include "frontend.h"
#undef operator
};

#include "utility/SemanticAnalysis.hpp"
#include "utility/Converter.hpp"
#include "utility/ASTOptimizer.hpp"

// backend
#include "Assembly.hpp"
#include "AssemblyPrinter.hpp"
#include "InstructionSelector.hpp"


/**
 * Compiler options:
 * m - output memory map to stderr
 * i - output ir to stderr
 * s - output simplified ir to stderr
 */
int main(int argc, const char** argv) {
    bool memoryMapOpt = false, irOpt = false, simpleIrOpt = false;
    int opt;
    while ((opt = getopt(argc, (char**) argv, ":mis")) != -1) {
        switch (opt) {
            case 'm': 
                memoryMapOpt = true;
                break;
            case 'i':
                irOpt = true;
                break;
            case 's':
                simpleIrOpt = true;
                break;
            default: break;
        }
    }

    if (optind + 1 >= argc) {
        std::cout << "Usage: " << argv[0] << " <infile> <outfile>" << std::endl;
        return 1;
    }

    const char* inputFile = argv[optind];
    const char* outputFile = argv[++optind];
    ASTProgram* astProgram = parser_parseFile(inputFile);
    if (astProgram == nullptr) {
        std::cout << "Compilation error: unable to obtain ast representation" << std::endl;
        return 2;
    }

    GlobalSymbolTable* globalSymbolTable;
    ASTDeclarationList* declarationList = astProgram->declarations;
    BaseBlock* startBlock;
    try {
        semanticanalysis::analyze(declarationList);
        globalSymbolTable = irconverter::symbolTableFrom(declarationList);
        semanticanalysis::analyze(astProgram->start, globalSymbolTable);
        std::cerr << "Compilation status: semantic analysis complete" << std::endl;

        astoptimizer::optimize(astProgram);
        std::cerr << "Compilation status: ast optimization complete" << std::endl;

        startBlock = irconverter::convertList(astProgram->start, globalSymbolTable);
        std::cerr << "Compilation status: hybrid ir conversion complete" << std::endl;

        GlobalSymbolTable& globalSymbolTableRef = *globalSymbolTable;
        ConstantTable constantTable;
        std::list<ThreeAddressCodeBlock> llir = isaselector::expand(*startBlock, globalSymbolTableRef, constantTable);

        if (memoryMapOpt) {
            std::cerr << "Memory map: " << std::endl;
            for (auto& it : globalSymbolTableRef.allRecords()) {
                std::cerr << it.second.name << " : " << it.second.offset << ", ";
            }
            std::cerr << std::endl << std::endl;
        }

        if (irOpt) {
            std::cerr << "Full IR: " << std::endl;
            for (auto& tacBlock : llir) {
                std::cerr << "l" << tacBlock.id() << ":" << std::endl;
                for (auto& tac: tacBlock.codes()) {
                    std::cerr << tac << std::endl;
                }
            }
            std::cerr << std::endl;
        }

        for (auto& tacBlock : llir) {
            isaselector::simplify(tacBlock, globalSymbolTableRef);
        }
        
        if (simpleIrOpt) {
            std::cerr << "Simplified IR: " << std::endl;
            for (auto& tacBlock : llir) {
                std::cerr << "l" << tacBlock.id() << ":" << std::endl;
                for (auto& tac: tacBlock.codes()) {
                    std::cerr << tac << std::endl;
                }
            }
            std::cerr << std::endl;
        }
        std::cerr << "Compilation status: hybrid ir simplification complete" << std::endl;

        AssemblyBlock asmBlock;
        JumpTable jumpTable;
        isaselector::initializationBlock(asmBlock, globalSymbolTableRef);
        for (auto& tacBlock : llir) {
            isaselector::match(asmBlock, jumpTable, tacBlock, globalSymbolTableRef, constantTable);
        }
        isaselector::terminationBlock(asmBlock, jumpTable);
        isaselector::applyJumpTable(asmBlock, jumpTable);
        std::cerr << "Compilation status: code generation complete" << std::endl;

        asmprinter::printToFile(outputFile, asmBlock);
    } catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return 3;
    }

    return 0;
}
