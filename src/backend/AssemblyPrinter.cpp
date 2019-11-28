//
// Created by Kacper Raczy on 2019-11-28.
//

#include "AssemblyPrinter.hpp"

void asmprinter::printToFile(const std::string &filename, AssemblyBlock& asmBlock) {
    std::ofstream file;
    file.exceptions( std::ofstream::failbit | std::ofstream::badbit );
    file.open(filename, std::ios::out | std::ios::trunc );
    printToFile(file, asmBlock);
    file.close();
}

void asmprinter::printToFile(std::ofstream &file, AssemblyBlock& asmBlock) {
    for (const Assembly& assembly : asmBlock) {
        std::string str = assembly.toString();
        file << str << std::endl;
    }
}
