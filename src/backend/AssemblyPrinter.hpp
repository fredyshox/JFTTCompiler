//
// Created by Kacper Raczy on 2019-11-28.
//

#ifndef backend_assembly_printer_hpp
#define backend_assembly_printer_hpp

#include <iostream>
#include <fstream>
#include "InstructionSelector.hpp"

namespace asmprinter {
    void printToFile(const std::string& filename, AssemblyBlock& asmBlock) noexcept(false);
    void printToFile(std::ofstream& file, AssemblyBlock& asmBlock) noexcept(false);
}

#endif /* backend_assembly_printer_hpp */
