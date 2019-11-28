//
// Created by Kacper Raczy on 2019-11-26.
//

#include <iostream>
#include <exception>
#include <list>

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

int main(int argc, const char** argv) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <infile> <outfile>" << std::endl;
        return 1;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];
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
        std::list<ThreeAddressCodeBlock> llir = isaselector::expand(*startBlock, globalSymbolTableRef);

        std::cerr << "Memory map: " << std::endl;
        for (auto& it : globalSymbolTableRef.allRecords()) {
            std::cerr << it.second.name << " : " << it.second.offset << ", ";
        }
        std::cerr << std::endl << std::endl;

        std::cerr << "Full: " << std::endl;
        for (auto& tacBlock : llir) {
            std::cerr << "l" << tacBlock.id() << ":" << std::endl;
            for (auto& tac: tacBlock.codes()) {
                std::cerr << tac.toString() << std::endl;
            }
            isaselector::simplify(tacBlock, globalSymbolTableRef);
        }
        std::cerr << std::endl;
        std::cerr << "Simplified: " << std::endl;
        for (auto& tacBlock : llir) {
            std::cerr << "l" << tacBlock.id() << ":" << std::endl;
            for (auto& tac: tacBlock.codes()) {
                std::cerr << tac.toString() << std::endl;
            }
        }
        std::cerr << "Compilation status: hybrid ir simplification complete" << std::endl;

        AssemblyBlock asmBlock;
        JumpTable jumpTable;
        isaselector::initializationBlock(asmBlock, globalSymbolTableRef);
        for (auto& tacBlock : llir) {
            isaselector::match(asmBlock, jumpTable, tacBlock, globalSymbolTableRef);
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
