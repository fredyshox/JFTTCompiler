//
// Created by Kacper Raczy on 2019-10-02.
//

#ifndef backend_instruciton_selector_hpp
#define backend_instruciton_selector_hpp

#include "blocks/ControlFlowBlock.hpp"
#include "blocks/ConditionBlock.hpp"
#include "blocks/WhileLoopBlock.hpp"
#include "blocks/ForLoopBlock.hpp"
#include "SymbolTable.hpp"
#include "Assembly.hpp"
#include <vector>

using AssemblyBlock = std::vector<Assembly>;

namespace isaselector {
    /**
     * Expands control flow blocks into basic three address code blocks.
     * @param program program to expand
     */
    ThreeAddressCodeBlock* expand(BaseBlock& program);

    /**
     * Simplifies three address code block.
     * @param block block to simplify
     */
    void simplify(ThreeAddressCodeBlock& block, SymbolTable& table);

    /**
     * Matches three address code to target isa instructions.
     * @param block block to match instructions to
     */
    void match(ThreeAddressCodeBlock& block);

    // Utility
    AssemblyBlock matchStdin(Operand* operand, SymbolTable& table);
    AssemblyBlock matchStdout(Operand* operand, SymbolTable& table);
    AssemblyBlock loadToP0(Operand* operand, SymbolTable& table) noexcept(false);
    AssemblyBlock loadToP0(int64_t value);
    AssemblyBlock storeP0(Operand* operand, SymbolTable& table) noexcept(false);
}

#endif /* backend_instruciton_selector_hpp */
